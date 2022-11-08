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
	if (!Init())
		return false;

	LPDIRECTSOUNDNOTIFY pDSNotify = nullptr;
	LPDSBPOSITIONNOTIFY pDSPositionNotify = new DSBPOSITIONNOTIFY[m_nBufNum]();
	if (!pDSPositionNotify)
		return false;
	m_phEvents = new HANDLE[m_nBufNum]();


	if (FAILED(m_secondaryBuf->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&pDSNotify))) {
		delete[] pDSPositionNotify;
		return false;
	}

	for (int i = 0; i < m_nBufNum; i++)
	{
		m_phEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		pDSPositionNotify[i].dwOffset = (m_bytesPerNotifySize * i) + (m_bytesPerNotifySize >> 1);
		pDSPositionNotify[i].hEventNotify = m_phEvents[i];
	}

	if (FAILED(pDSNotify->SetNotificationPositions(m_nBufNum, pDSPositionNotify))) {
		pDSNotify->Release();
		delete[] pDSPositionNotify;
		return false;
	}

	pDSNotify->Release();
	delete[] pDSPositionNotify;

	if (FAILED(m_secondaryBuf->Play(0, 0, DSBPLAY_LOOPING)))
		return false;

	m_bRun = true;
	m_threadPlay = std::thread(&DSPlayer::PlayThread, this);

	return true;
}

void DSPlayer::PushPCMBuf(uint8_t* pBuf, int nSize)
{
	std::lock_guard<std::mutex> lock(m_mux);

	PCM_BUF pcm = { 0 };
	pcm.size = nSize;
	pcm.buf = new uint8_t[nSize]();
	memcpy(pcm.buf, pBuf, nSize);

	m_pcmQueue.push(pcm);
}

bool DSPlayer::Init()
{
	DSBUFFERDESC dsBufDesc = { 0 };
	WAVEFORMATEX waveFormat = { 0 };

	if (FAILED(DirectSoundCreate(nullptr, &m_device, nullptr)))
		return false;

	if (FAILED(m_device->SetCooperativeLevel(::GetDesktopWindow(), DSSCL_PRIORITY)))
		return false;

	dsBufDesc.dwSize = sizeof(DSBUFFERDESC);
	dsBufDesc.dwFlags = DSBCAPS_PRIMARYBUFFER; // 表示主缓冲区
	dsBufDesc.dwBufferBytes = 0;			// 主缓冲区必须设置为0
	dsBufDesc.lpwfxFormat = nullptr;		// 主缓冲区必须设置为null

	// 创建主缓冲区
	if (FAILED(m_device->CreateSoundBuffer(&dsBufDesc, &m_primaryBuf, nullptr))) 
		return false;

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 2;			// 声道数
	waveFormat.nSamplesPerSec = 44100;	// 采样频率
	waveFormat.wBitsPerSample = 16;		// 采样精度
	waveFormat.nBlockAlign = waveFormat.wBitsPerSample * waveFormat.nChannels / 8; // 采样块对齐大小
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;// 平均每秒采样字节数

	// 设置缓冲区格式
	if (FAILED(m_primaryBuf->SetFormat(&waveFormat))) 
		return false;

	// 创建次缓冲区
	m_bytesPerNotifySize = waveFormat.nAvgBytesPerSec * 60 / 1000;
	dsBufDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME;
	dsBufDesc.dwBufferBytes = m_bytesPerNotifySize * m_nBufNum;
	dsBufDesc.lpwfxFormat = &waveFormat;
	if (FAILED(m_device->CreateSoundBuffer(&dsBufDesc, &m_secondaryBuf, nullptr)))
		return false;

	m_bytesNotifyPtr = new uint8_t[m_bytesPerNotifySize*3]();

	return true;
}

void DSPlayer::PlayThread()
{
	LPVOID pAudio1 = nullptr, pAudio2 = nullptr;
	DWORD dwAudio1 = 0, dwAudio2 = 0;

	while (m_bRun)
	{
		DWORD dwRet = WaitForMultipleObjects(m_nBufNum, m_phEvents, FALSE, INFINITE);

		if (m_mux.try_lock())
		{
			if (!m_pcmQueue.empty())
			{
				PCM_BUF pcm_buf = m_pcmQueue.front();

				if (FAILED(m_secondaryBuf->Lock(0, m_bytesPerNotifySize, &pAudio1, &dwAudio1, 0, 0, DSBLOCK_ENTIREBUFFER))) {
					printf("m_secondaryBuf->Lock failed.\n");
					continue;
				}

				if (m_offset < m_bytesPerNotifySize)
				{
					memcpy(m_bytesNotifyPtr + m_offset, pcm_buf.buf, pcm_buf.size);
					m_offset += pcm_buf.size;

					delete[] pcm_buf.buf;
					m_pcmQueue.pop();
				}
				else
				{
					int iOffset = m_offset - m_bytesPerNotifySize;

					memcpy(pAudio1, m_bytesNotifyPtr, m_bytesPerNotifySize);

					m_offset -= m_bytesPerNotifySize;
				}

			

				m_secondaryBuf->Unlock(pAudio1, dwAudio1, 0, 0);
			}

			m_mux.unlock();
		}		
	}
}
