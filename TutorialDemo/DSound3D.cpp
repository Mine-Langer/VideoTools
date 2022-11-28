#include "DSound3D.h"

DSound3D::DSound3D()
{
	Init();
}

DSound3D::~DSound3D()
{
}

bool DSound3D::Open(TCHAR* szFile)
{
	m_waveFile.Open(szFile, nullptr, WAVEFILE_READ);
	WAVEFORMATEX* pwfx = m_waveFile.WaveFormat();

	DSBUFFERDESC dsbd = { 0 };
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = m_waveFile.Size();
	dsbd.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
	dsbd.lpwfxFormat = pwfx;
	if (FAILED(m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuf, nullptr)))
		return false;

	for (int i = 0; i < MAX_AUDIO_BUF; i++)
	{
		m_dsPosNotify[i].dwOffset = i * BUFFER_NOTIFY_SIZE;
		m_dsPosNotify[i].hEventNotify = m_hEvent[i];
	}

	if (FAILED(m_pDSBuf->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&m_pDS3DBuf)))
		return false;

	m_ds3DBuffer.dwSize = sizeof(DS3DBUFFER);
	m_pDS3DBuf->GetAllParameters(&m_ds3DBuffer);

	m_ds3DBuffer.dwMode = DS3DMODE_HEADRELATIVE;
	m_pDS3DBuf->SetAllParameters(&m_ds3DBuffer, DS3D_IMMEDIATE);

	if (FAILED(m_pDSBuf->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&m_pDSNotify)))
		return false;

	m_pDSNotify->SetNotificationPositions(MAX_AUDIO_BUF, m_dsPosNotify);
	m_pDSNotify->Release();

	m_ds3DListener.flDopplerFactor = 50;
	m_ds3DListener.flRolloffFactor = 50;
	if (m_pDS3DListener)
	{
		m_pDS3DListener->SetAllParameters(&m_ds3DListener, DS3D_DEFERRED);
		m_pDS3DListener->CommitDeferredSettings();
	}

	m_ds3DBuffer.flMinDistance = 30;
	m_ds3DBuffer.flMaxDistance = 100;
	if (m_pDS3DBuf)
		m_pDS3DBuf->SetAllParameters(&m_ds3DBuffer, DS3D_DEFERRED);

	return true;
}

bool DSound3D::Init()
{
	LPDIRECTSOUNDBUFFER pDSBPrimary = nullptr;
	DSBUFFERDESC dsbdesc = { 0 };

	if (FAILED(DirectSoundCreate8(nullptr, &m_pDS, nullptr)))
		return false;

	if (FAILED(m_pDS->SetCooperativeLevel(GetForegroundWindow(), DSSCL_PRIORITY)))
		return false;
	
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	if (FAILED(m_pDS->CreateSoundBuffer(&dsbdesc, &pDSBPrimary, nullptr)))
		return false;

	if (FAILED(pDSBPrimary->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&m_pDS3DListener)))
		return false;

	WAVEFORMATEX wfx = { 0 };
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
	if (FAILED(pDSBPrimary->SetFormat(&wfx)))
		return false;

	for (int i = 0; i < MAX_AUDIO_BUF; i++)
	{
		m_hEvent[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	return true;
}
