#include "PcmPlay.h"
/**********************************************************************************/
	// 1,����Dsound�豸����
	// 2���趨�豸��Э������
	// 3������DSound������
	// 4���������ݵ�������
	// 5����������
	// 6��ֹͣ����
	// 7���˳�Ӧ��
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
	dsBufDesc.dwFlags = DSBCAPS_PRIMARYBUFFER; // ��ʾ��������
	dsBufDesc.dwBufferBytes = 0;			// ����������������Ϊ0
	dsBufDesc.lpwfxFormat = nullptr;		// ����������������Ϊnull

	// ������������
	if (FAILED(m_device->CreateSoundBuffer(&dsBufDesc, &m_primaryBuf, nullptr))) 
		return false;

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 2;			// ������
	waveFormat.nSamplesPerSec = 44100;	// ����Ƶ��
	waveFormat.wBitsPerSample = 16;		// ��������
	waveFormat.nBlockAlign = waveFormat.wBitsPerSample * waveFormat.nChannels / 8; // ����������С
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;// ƽ��ÿ������ֽ���

	// ���û�������ʽ
	if (FAILED(m_primaryBuf->SetFormat(&waveFormat))) 
		return false;

	// �����λ�����
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

///////////////////////////////////////////////////////////////////////////////
/*
���ܣ��ͷ���Դ
*/
int unprepare(Player* ds) {
	printf("unprepare in.\n");
	if (!ds) {
		return -1;
	}
	/*������*/
	if (ds->primaryBuffer) {
		IDirectSoundBuffer_Release(ds->primaryBuffer);
		ds->primaryBuffer = NULL;
	}
	/*��������*/
	if (ds->secondaryBuffer) {
		IDirectSoundBuffer_Release(ds->secondaryBuffer);
		ds->secondaryBuffer = NULL;
	}
	/*�����豸����*/
	if (ds->device) {
		IDirectSound_Release(ds->device);
		ds->device = NULL;
	}
	/*֪ͨ�¼�*/
	for (size_t i = 0; i < sizeof(ds->notifEvents) / sizeof(ds->notifEvents[0]); i++) {
		if (ds->notifEvents[i]) {
			CloseHandle(ds->notifEvents[i]);
			ds->notifEvents[i] = NULL;
		}
	}
	return 0;
}

/*
���ܣ�¼��׼��
*/
int prepare(Player* ds) {
	printf("prepare in.\n");
	HRESULT hr;
	HWND hWnd;
	WAVEFORMATEX wfx = { 0 };
	DSBUFFERDESC dsbd = { 0 };

	if (ds->device || ds->primaryBuffer || ds->secondaryBuffer) {
		return -1;
	}

	/* ���������豸 */
	if ((hr = DirectSoundCreate(NULL, &ds->device, NULL) != DS_OK)) {
		return -3;
	}

	/* ����Э������ */
	if ((hWnd = GetForegroundWindow()) || (hWnd = GetDesktopWindow()) || (hWnd = GetConsoleWindow())) {
		if ((hr = IDirectSound_SetCooperativeLevel(ds->device, hWnd, DSSCL_PRIORITY)) != DS_OK) {
			return -4;
		}
	}

	/* ����������������ʽ���� */
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample / 8);
	wfx.nAvgBytesPerSec = (wfx.nSamplesPerSec * wfx.nBlockAlign);

	/* ����֪ͨ���ݰ���С�����ٻ����� */
	ds->bytes_per_notif_size = ((wfx.nAvgBytesPerSec * 60) / 1000);
	if (!(ds->bytes_per_notif_ptr = (uint8_t*)realloc(ds->bytes_per_notif_ptr, ds->bytes_per_notif_size))) {
		//DEBUG_ERROR("Failed to allocate buffer with size = %u", _bytes_per_notif_size);
		return -5;
	}

	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat = NULL;

	if ((hr = IDirectSound_CreateSoundBuffer(ds->device, &dsbd, &ds->primaryBuffer, NULL)) != DS_OK) {
		return -6;
	}

	if ((hr = IDirectSoundBuffer_SetFormat(ds->primaryBuffer, &wfx)) != DS_OK) {
		return -7;
	}

	/* �����λ���������ʽ���� */
	dsbd.dwFlags = (DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME);
	dsbd.dwBufferBytes = (DWORD)(20 * ds->bytes_per_notif_size);
	dsbd.lpwfxFormat = &wfx;

	if ((hr = IDirectSound_CreateSoundBuffer(ds->device, &dsbd, &ds->secondaryBuffer, NULL)) != DS_OK) {
		return -8;
	}

#if 0
	/* �������� [-10000,0]*/
	if (IDirectSoundBuffer_SetVolume(_secondaryBuffer, 0/*_convert_volume(0)*/) != DS_OK) {
		printf("setVolume error\n");
	}
#endif

	return 0;
}

/*
���ܣ���ʼ¼��
*/
int startPlayer(Player* ds) {
	printf("startPlayer in.\n");
	HRESULT hr;
	LPDIRECTSOUNDNOTIFY lpDSBNotify;
	DSBPOSITIONNOTIFY pPosNotify[20] = { 0 };

	static DWORD dwMajorVersion = -1;

	if (!ds) {
		return -1;
	}
	if (ds->started) {
		return 0;
	}

	// Get OS version
	if (dwMajorVersion == -1) {
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#pragma warning (disable: 4996) //���ξ��棬GetVersionEx������
		GetVersionEx(&osvi);
		dwMajorVersion = osvi.dwMajorVersion;
	}
	/*����׼��*/
	if (prepare(ds)) {
		return -2;
	}

	if (!ds->device || !ds->primaryBuffer || !ds->secondaryBuffer) {
		return -3;
	}

	if ((hr = IDirectSoundBuffer_QueryInterface(ds->secondaryBuffer, IID_IDirectSoundNotify, (LPVOID*)&lpDSBNotify)) != DS_OK) {
		return -4;
	}

	/* ����¼�֪ͨ�� */
	for (size_t i = 0; i < 20; i++) {
		ds->notifEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		/*��Windows Vista�����߰汾�Ļ�������ʼ������֪ͨ��ƫ����������XP��֮ǰ�Ļ�������һ������֪ͨ��ƫ����
		win7 dwMajorVersion = 6*/
		pPosNotify[i].dwOffset = (DWORD)((ds->bytes_per_notif_size * i) + (dwMajorVersion > 5 ? (ds->bytes_per_notif_size >> 1) : 1));
		pPosNotify[i].hEventNotify = ds->notifEvents[i];
	}
	if ((hr = IDirectSoundNotify_SetNotificationPositions(lpDSBNotify, 20, pPosNotify)) != DS_OK) {
		IDirectSoundNotify_Release(lpDSBNotify);
		return -5;
	}

	if ((hr = IDirectSoundNotify_Release(lpDSBNotify))) {

	}

	/* ��ʼ���Ż�����
	���λ����������������͵���������,�������������л��,����䵽���������Զ�����.*/
	if ((hr = IDirectSoundBuffer_Play(ds->secondaryBuffer, 0, 0, DSBPLAY_LOOPING)) != DS_OK) {
		return -6;
	}


	if (openFile(ds)) {
		return -2;
	}


	ds->tid[0] = ::CreateThread(NULL, 0, playerThreadImpl, ds, 0, NULL);
	if (!ds->tid[0]) {
		printf("thread create error.\n");
	}
	/* �����߳� */
	ds->started = true;

	return 0;
}

/*
���ܣ���ͣ����
*/
int suspendPlayer(Player* ds) {
	return 0;
}

/*
���ܣ����²���
*/
int resumePlayer(Player* ds) {
	return 0;
}

/*
���ܣ�ֹͣ����
*/
int stopPlayer(Player* ds) {
	printf("stopPlayer in.\n");
	HRESULT hr;

	if (!ds || !ds->started) {
		return 0;
	}
	ds->started = false;
	if (ds->tid[0]) {
		if (0 == (WaitForSingleObject(ds->tid[0], INFINITE) == WAIT_FAILED) ? -1 : 0) {
			::CloseHandle(ds->tid[0]);
			ds->tid[0] = NULL;
		}
	}

	if ((hr = IDirectSoundBuffer_Stop(ds->secondaryBuffer)) != DS_OK) {

	}

	if ((hr = IDirectSoundBuffer_SetCurrentPosition(ds->secondaryBuffer, 0)) != DS_OK) {

	}

	if (closeFile(ds)) {
		return -2;
	}

	// unprepare
	// will be prepared again before calling next start()
	unprepare(ds);

	return 0;
}

/*
���ܣ��߳�ִ����
*/
DWORD WINAPI playerThreadImpl(LPVOID params) {
	printf("playerThreadImpl in.\n");
	HRESULT hr;
	LPVOID lpvAudio1, lpvAudio2;
	DWORD dwBytesAudio1, dwBytesAudio2, dwEvent;
	static const DWORD dwWriteCursor = 0;
	size_t out_size = 0;

	Player* ds = (Player*)(params);
	if (!ds) {
		return NULL;
	}
	SetThreadPriority(GetCurrentThread(), THREAD_BASE_PRIORITY_MAX);

	while (ds->started) {
		dwEvent = WaitForMultipleObjects(20, ds->notifEvents, FALSE, INFINITE);
		if (!ds->started) {
			break;
		}
		// lock
		if (hr = IDirectSoundBuffer_Lock(ds->secondaryBuffer,
			dwWriteCursor/* Ignored because of DSBLOCK_FROMWRITECURSOR */,
			(DWORD)ds->bytes_per_notif_size,
			&lpvAudio1, &dwBytesAudio1,
			&lpvAudio2, &dwBytesAudio2,
			DSBLOCK_FROMWRITECURSOR) != DS_OK) {
			printf("IDirectSoundBuffer_Lock error\n");
			continue;
		}


		if ((out_size = fread(ds->bytes_per_notif_ptr, 1, ds->bytes_per_notif_size, ds->fp)) != ds->bytes_per_notif_size) {
			//ds->started = false;//ֹͣ����
			printf("player finish.\n");
			stopPlayer(ds);
		}

		if (out_size < ds->bytes_per_notif_size) {
			// fill with silence
			memset(&ds->bytes_per_notif_ptr[out_size], 0, (ds->bytes_per_notif_size - out_size));
		}
		if ((dwBytesAudio1 + dwBytesAudio2) == ds->bytes_per_notif_size) {
			memcpy(lpvAudio1, ds->bytes_per_notif_ptr, dwBytesAudio1);
			if (lpvAudio2 && dwBytesAudio2) {
				memcpy(lpvAudio2, &ds->bytes_per_notif_ptr[dwBytesAudio1], dwBytesAudio2);
			}
		}
		else {
			//DEBUG_ERROR("Not expected: %d+%d#%d", dwBytesAudio1, dwBytesAudio2, dsound->bytes_per_notif_size);
		}

		// unlock
		if ((hr = IDirectSoundBuffer_Unlock(ds->secondaryBuffer, lpvAudio1, dwBytesAudio1, lpvAudio2, dwBytesAudio2)) != DS_OK) {

		}
	}

	return NULL;
}


int openFile(Player* ds) {
	printf("openFile in.\n");
	if (!ds) {
		return -1;
	}
	if (!(ds->fp)) {
		fopen_s(&(ds->fp), "./Titan.pcm", "rb");
		if (ds->fp == NULL) {
			printf("cannot open PCM file.\n");
			return -2;
		}
	}
	return 0;
}

int closeFile(Player* ds) {
	printf("closeFile in.\n");
	if (!ds) {
		return -1;
	}
	if (ds->fp) {
		fclose(ds->fp);
		ds->fp = NULL;
	}
	return 0;
}