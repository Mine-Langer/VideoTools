#pragma once
#include <Windows.h>
#include <tchar.h>
#include <dsound.h>
#include <mmsystem.h>
#include <cinttypes>
#include <queue>
#include <thread>
#include <mutex>

#define BUF_NUM  20

class DxAudioPlayer
{
public:
	DxAudioPlayer();
	~DxAudioPlayer();

	bool Open(int nChannel, int nSamples);

	void PushPCM(uint8_t* pBuf, uint32_t uSize);

	void Stop();

private:

	void OnPlayProc();

private:

	LPDIRECTSOUND		m_pDevice = nullptr;
	LPDIRECTSOUNDBUFFER	m_pMainBuff = nullptr;
	LPDIRECTSOUNDBUFFER m_pSecondBuff = nullptr;
	DSBPOSITIONNOTIFY	m_posNotifyArr[BUF_NUM];
	HANDLE				m_hNotifyEvts[BUF_NUM];

	WAVEFORMATEX	m_WaveFormat;

	uint32_t		m_notifySize = 0;

	bool			m_bRun = false;
	std::thread		m_tPlay;
};

