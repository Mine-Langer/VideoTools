#pragma once
#include "WaveFile.h"

#define MAX_AUDIO_BUF 3

class DSound3D
{
public:


private:
	LPDIRECTSOUND8			m_pDS = nullptr;	// DirectSound����ָ��
	LPDIRECTSOUNDBUFFER		m_pDSBuf = nullptr;	// ����������ָ��
	LPDIRECTSOUND3DBUFFER	m_pDS3DBuf = nullptr; // 3D ��Դ����ָ��
	LPDIRECTSOUND3DLISTENER	m_pDS3DListener = nullptr; // 3D����ָ��
	DS3DBUFFER				m_ds3DBuffer;	// 3D buffer
	DS3DLISTENER			m_ds3DListener; // 
	CWaveFile				m_waveFile;

	LPDIRECTSOUNDNOTIFY8	m_pDSNotify = nullptr;
	DSBPOSITIONNOTIFY		m_dsPosNotify[MAX_AUDIO_BUF];
};

