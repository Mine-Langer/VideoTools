#include "PcmPlay.h"
/**********************************************************************************/
	// 1,创建Dsound设备对象
	// 2，设定设备的协调级别
	// 3，创建DSound缓冲区
	// 4，加载数据到缓冲区
	// 5，播放声音
	// 6，停止播放
	// 7，退出应用
/**********************************************************************************/

DSPlayer::DSPlayer()
{
}

DSPlayer::~DSPlayer()
{
}

bool DSPlayer::Start()
{
	LPDIRECTSOUNDNOTIFY lpDSNotify = nullptr;
	DSBPOSITIONNOTIFY arrPosNotify[10] = { 0 };

	if (!Init())
		return false;

	if (FAILED(m_secondaryBuf->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&lpDSNotify)))
		return false;

	for (int i = 0; i < m_nBufNum; i++)
	{
		m_phEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		arrPosNotify[i].dwOffset = (m_bytesPerNotifySize * i) + (m_bytesPerNotifySize >> 1);
		arrPosNotify[i].hEventNotify = m_phEvents[i];
	}

	if (FAILED(lpDSNotify->SetNotificationPositions(m_nBufNum, arrPosNotify))) {
		lpDSNotify->Release();
		return false;
	}

	lpDSNotify->Release();

	if (FAILED(m_secondaryBuf->Play(0, 0, DSBPLAY_LOOPING)))
		return false;

	//m_hFile = CreateFile(_T("Titan.pcm"), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	//if (m_hFile == INVALID_HANDLE_VALUE)
	//	return false;

	m_bRun = true;
	m_threadPlay = std::thread(&DSPlayer::PlayThread, this);

	return true;
}

void DSPlayer::PushPCMBuf(uint8_t* pBuf, int nSize)
{
	std::lock_guard<std::mutex> lock(m_mux);
	PCM_BUF pcm = { 0 };
	
	pcm.buf = new uint8_t[nSize]();
	pcm.size = nSize;
	memcpy(pcm.buf, pBuf, nSize);

	m_pcmQueue.push(pcm);
}

void DSPlayer::Stop()
{
	m_bRun = false;
	if (m_threadPlay.joinable())
		m_threadPlay.join();

	m_secondaryBuf->Stop();

	m_secondaryBuf->SetCurrentPosition(0);

	Release();
}

bool DSPlayer::Init()
{
	// 创建播放设备
	if (FAILED(DirectSoundCreate(nullptr, &m_pDevice, nullptr)))
		return false;

	// 设置协调级别
	if (FAILED(m_pDevice->SetCooperativeLevel(::GetForegroundWindow(), DSSCL_PRIORITY)))
		return false;

	// 创建主缓冲区
	WAVEFORMATEX waveFormat = { 0 };
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 2;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

	m_bytesPerNotifySize = waveFormat.nAvgBytesPerSec * 60 / 1000;
	m_bytesNotifyPtr = new uint8_t[m_bytesPerNotifySize*3]();

	// 创建主缓冲区
	DSBUFFERDESC dsBufDesc = { 0 };
	dsBufDesc.dwSize = sizeof(DSBUFFERDESC);
	dsBufDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (FAILED(m_pDevice->CreateSoundBuffer(&dsBufDesc, &m_primaryBuf, nullptr)))
		return false;

	if (FAILED(m_primaryBuf->SetFormat(&waveFormat)))
		return false;

	// 创建次缓冲区
	dsBufDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
	dsBufDesc.dwBufferBytes = m_nBufNum * m_bytesPerNotifySize;
	dsBufDesc.lpwfxFormat = &waveFormat;
	if (FAILED(m_pDevice->CreateSoundBuffer(&dsBufDesc, &m_secondaryBuf, nullptr)))
		return false;

	return true;
}

void DSPlayer::Release()
{
	if (m_primaryBuf)
	{
		m_primaryBuf->Release();
		m_primaryBuf = nullptr;
	}

	if (m_secondaryBuf)
	{
		m_secondaryBuf->Release();
		m_secondaryBuf = nullptr;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}

	for (int i = 0; i < m_nBufNum; i++)
	{
		if (m_phEvents[i])
		{
			CloseHandle(m_phEvents[i]);
			m_phEvents[i] = nullptr;
		}
	}
}

void DSPlayer::PlayThread()
{
	LPVOID lpAudio1, lpAudio2;
	DWORD dwAudio1, dwAudio2;
	DWORD dwBytes = 0;
	int nIdx = 0;
	DWORD dwOffset = 0, dwSendSize = 0;;

	while (m_bRun)
	{
		DWORD dwRet = WaitForMultipleObjects(m_nBufNum, m_phEvents, FALSE, INFINITE);

		if (FAILED(m_secondaryBuf->Lock(0, m_bytesPerNotifySize, &lpAudio1, &dwAudio1, &lpAudio2, &dwAudio2, DSBLOCK_FROMWRITECURSOR)))
			continue;
		
		//if (!ReadFile(m_hFile, m_bytesNotifyPtr, m_bytesPerNotifySize, &dwBytes, nullptr))
		//	break;
		while (true)
		{
			if (dwOffset < m_bytesPerNotifySize)
			{
				std::lock_guard<std::mutex> lock(m_mux);
				if (!m_pcmQueue.empty())
				{
					PCM_BUF pcmbuf = m_pcmQueue.front();
					memcpy(m_bytesNotifyPtr + dwOffset, pcmbuf.buf, pcmbuf.size);
					delete[] pcmbuf.buf;
					m_pcmQueue.pop();
					dwOffset += pcmbuf.size;
				}
			}
			else
			{
				memcpy(lpAudio1, m_bytesNotifyPtr, m_bytesPerNotifySize);
				dwOffset -= m_bytesPerNotifySize;
				memcpy(m_bytesNotifyPtr, m_bytesNotifyPtr + m_bytesPerNotifySize, dwOffset);
				break;
			}
		}
		//printf("read size:%d, index:%d\n", dwBytes, nIdx++);

		//if (dwAudio1 + dwAudio2 == m_bytesPerNotifySize)
		//{
		//	memcpy(lpAudio1, m_bytesNotifyPtr, dwAudio1);
		//	if (lpAudio2 && dwAudio1)
		//		memcpy(lpAudio2, m_bytesNotifyPtr + dwAudio1, dwAudio2);
		//}

		m_secondaryBuf->Unlock(lpAudio1, dwAudio1, lpAudio2, dwAudio2);
	}
	CloseHandle(m_hFile);

	Stop();
}


