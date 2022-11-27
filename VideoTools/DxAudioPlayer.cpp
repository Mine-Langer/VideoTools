#include "DxAudioPlayer.h"
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dsound.lib")
#pragma comment (lib, "dxguid.lib")

DxAudioPlayer::DxAudioPlayer()
{
}

DxAudioPlayer::~DxAudioPlayer()
{
}

bool DxAudioPlayer::Open(int nChannel, int nSamples)
{
	m_WaveFormat.cbSize = sizeof(WAVEFORMATEX);
	m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat.nChannels = nChannel;
	m_WaveFormat.nSamplesPerSec = nSamples;
	m_WaveFormat.wBitsPerSample = 16;
	m_WaveFormat.nBlockAlign = m_WaveFormat.wBitsPerSample * m_WaveFormat.nChannels / 8;
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;

	m_notifySize = m_WaveFormat.nAvgBytesPerSec * 60 / 1000;
	m_bytesNotifyPtr = new uint8_t[m_notifySize * 10]();

	if (FAILED(DirectSoundCreate(nullptr, &m_pDevice, nullptr)))
		return false;

	if (FAILED(m_pDevice->SetCooperativeLevel(::GetForegroundWindow(), DSSCL_PRIORITY)))
		return false;

	DSBUFFERDESC bufDesc = { 0 };
	bufDesc.dwSize = sizeof(DSBUFFERDESC);
	bufDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (FAILED(m_pDevice->CreateSoundBuffer(&bufDesc, &m_pMainBuff, nullptr)))
		return false;

	if (FAILED(m_pMainBuff->SetFormat(&m_WaveFormat)))
		return false;

	bufDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
	bufDesc.dwBufferBytes = BUF_NUM * m_notifySize;
	bufDesc.lpwfxFormat = &m_WaveFormat;
	if (FAILED(m_pDevice->CreateSoundBuffer(&bufDesc, &m_pSecondBuff, nullptr)))
		return false;

	LPDIRECTSOUNDNOTIFY lpDsNotify = nullptr;
	if (FAILED(m_pSecondBuff->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&lpDsNotify)))
		return false;

	for (int i = 0; i < BUF_NUM; i++)
	{
		m_posNotifyArr[i].dwOffset = (m_notifySize * i) + (m_notifySize >> i);
		m_posNotifyArr[i].hEventNotify = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_hNotifyEvts[i] = m_posNotifyArr[i].hEventNotify;
	}

	if (FAILED(lpDsNotify->SetNotificationPositions(BUF_NUM, m_posNotifyArr)))
	{
		lpDsNotify->Release();
		return false;
	}

	lpDsNotify->Release();

	if (FAILED(m_pSecondBuff->Play(0, 0, DSBPLAY_LOOPING)))
		return false;

	m_bRun = true;
	m_tPlay = std::thread(&DxAudioPlayer::OnPlayProc, this);

	return true;
}

void DxAudioPlayer::PushPCM(uint8_t* pBuf, uint32_t uSize)
{
	PCM_BUF pcm = { 0 };

	pcm.buf = new uint8_t[uSize]();
	pcm.size = uSize;
	memcpy(pcm.buf, pBuf, uSize);

	m_pcmQueue.Push(pcm);
}

void DxAudioPlayer::OnPlayProc()
{
	LPVOID pvAudioPtr1, pvAudioPtr2;
	DWORD dwAudioBytes1, dwAudioBytes2;
	DWORD dwOffset = 0;
	while (m_bRun)
	{
		DWORD dwRet = WaitForMultipleObjects(BUF_NUM, m_hNotifyEvts, FALSE, INFINITE);

		if (FAILED(m_pSecondBuff->Lock(0, m_notifySize, &pvAudioPtr1, &dwAudioBytes1, &pvAudioPtr2, &dwAudioBytes2, DSBLOCK_FROMWRITECURSOR)))
			continue;

		while (true)
		{
			if (dwOffset < m_notifySize)
			{
				PCM_BUF pcmbuf;
				if (m_pcmQueue.Pop(pcmbuf))
				{
					memcpy(m_bytesNotifyPtr + dwOffset, pcmbuf.buf, pcmbuf.size);
					delete[] pcmbuf.buf;

					dwOffset += pcmbuf.size;
				}
			}
			else
			{
				memcpy(pvAudioPtr1, m_bytesNotifyPtr, dwAudioBytes1);
				memcpy(pvAudioPtr2, m_bytesNotifyPtr+ dwAudioBytes1, dwAudioBytes2);
				dwOffset -= m_notifySize;
				memcpy(m_bytesNotifyPtr, m_bytesNotifyPtr + m_notifySize, dwOffset);
				break;
			}
		}

		m_pSecondBuff->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
	}
}
