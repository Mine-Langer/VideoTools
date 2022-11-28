#pragma once
#include "WaveFile.h"

#define MAX_AUDIO_BUF 3
#define BUFFER_NOTIFY_SIZE 7680

class DSound3D
{
public:
	DSound3D();
	~DSound3D();

	bool Open(TCHAR* szFile);

private:
	bool Init();


private:
	LPDIRECTSOUND8			m_pDS = nullptr;	// DirectSound对象指针
	LPDIRECTSOUNDBUFFER		m_pDSBuf = nullptr;	// 辅助缓冲区指针
	LPDIRECTSOUND3DBUFFER	m_pDS3DBuf = nullptr; // 3D 声源对象指针
	LPDIRECTSOUND3DLISTENER	m_pDS3DListener = nullptr; // 3D听者指针
	DS3DBUFFER				m_ds3DBuffer;	// 3D buffer
	DS3DLISTENER			m_ds3DListener; // 
	CWaveFile				m_waveFile;
	BOOL					m_bPlaying = FALSE;

	LPDIRECTSOUNDNOTIFY8	m_pDSNotify = nullptr;
	DSBPOSITIONNOTIFY		m_dsPosNotify[MAX_AUDIO_BUF];
	HANDLE					m_hEvent[MAX_AUDIO_BUF];
	DWORD					m_dwNextWriteOffset = 0;
};

