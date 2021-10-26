#include "recoder.h"

CRecoder::CRecoder()
{

}

CRecoder::~CRecoder()
{

}

void CRecoder::SetConfig(int nSamples, int nBit, int nChannel)
{
	// �豸����
	int count = waveInGetNumDevs();

	// �豸����
	WAVEINCAPS waveInCaps = { 0 };
	MMRESULT mmResult = waveInGetDevCaps(0, &waveInCaps, sizeof(WAVEINCAPS));

	// ������Ƶ����ʽ
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
	waveFormat.nSamplesPerSec = nSamples;  // ������
	waveFormat.wBitsPerSample = nBit;     // ��������
	waveFormat.nChannels = nChannel;           // ������
	waveFormat.wFormatTag = WAVE_FORMAT_PCM; // ��Ƶ��ʽ
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample * waveFormat.nChannels) / 8; // �����
	waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec; // ��������

	// �����ڴ�
	BYTE* pBuffer1 = new BYTE[BUFFER_LEN]();
	BYTE* pBuffer2 = new BYTE[BUFFER_LEN]();
	file = new BYTE[512]();

	// ������Ƶͷ
	whdr_i1.lpData = (LPSTR)pBuffer1;
	whdr_i1.dwBufferLength = BUFFER_LEN;
	whdr_i1.dwBytesRecorded = 0;
	whdr_i1.dwUser = 0;
	whdr_i1.dwFlags = 0;
	whdr_i1.dwLoops = 1;
	whdr_i2.lpData = (LPSTR)pBuffer2;
	whdr_i2.dwBufferLength = BUFFER_LEN;
	whdr_i2.dwBytesRecorded = 0;
	whdr_i2.dwUser = 0;
	whdr_i2.dwFlags = 0;
	whdr_i2.dwLoops = 1;
}

bool CRecoder::Start()
{
	// ����¼��
	MMRESULT mmResult = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD_PTR)RecoderFunction, (DWORD_PTR)this, CALLBACK_FUNCTION);
	mmResult = waveInPrepareHeader(hWaveIn, &whdr_i1, sizeof(WAVEHDR));
	mmResult = waveInPrepareHeader(hWaveIn, &whdr_i2, sizeof(WAVEHDR));
	mmResult = waveInAddBuffer(hWaveIn, &whdr_i1, sizeof(WAVEHDR));
	mmResult = waveInAddBuffer(hWaveIn, &whdr_i2, sizeof(WAVEHDR));

	mmResult = waveInStart(hWaveIn);

	//////////////////////////////////////////////////////////////////////////
	waveInReset(hWaveIn);
	Sleep(50000);
	waveInClose(hWaveIn);

	HANDLE hWait = CreateEvent(nullptr, false, false, nullptr);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD_PTR)hWait, 0, CALLBACK_EVENT);

	// ����¼��
	whdr_o.lpData = (LPSTR)file;
	whdr_o.dwBufferLength = hasRecorded;
	whdr_o.dwBytesRecorded = hasRecorded;
	whdr_o.dwFlags = 0;
	whdr_o.dwLoops = 1;

	ResetEvent(hWait);
	waveOutPrepareHeader(hWaveOut, &whdr_o, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &whdr_o, sizeof(WAVEHDR));

	DWORD dw = WaitForSingleObject(hWait, INFINITE);
	if (dw == WAIT_OBJECT_0)
	{
		printf("over! \r\n");
		return true;
	}
	return false;
}

void CALLBACK CRecoder::RecoderFunction(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInst, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	CRecoder* pThis = (CRecoder*)dwInst;
	// ��ȡ��Ƶͷ
	PWAVEHDR pwhdr = (PWAVEHDR)dwParam1;

	// ������Ϣ
	switch (uMsg)
	{
	case WIM_OPEN:
		printf("Open Device succeed!\r\n");
		break;
	case WIM_DATA:
	{
		printf("Buffer has Complete!\r\n");
		DWORD bufLen = pwhdr->dwBufferLength;
		DWORD byteRecd = pwhdr->dwBytesRecorded;
		pThis->hasRecorded += byteRecd;

		pThis->file = (BYTE*)realloc(pThis->file, pThis->hasRecorded);
		if (pThis->file)
		{
			memcpy(&pThis->file[pThis->hasRecorded - byteRecd], pwhdr->lpData, byteRecd);
			printf("have save %d bytes.\r\n", pThis->hasRecorded);
		}
		if (pThis->recurr)
		{
			// ���뻺��
			waveInAddBuffer(hwi, pwhdr, sizeof(WAVEHDR));
		}
		break;
	}
	case WIM_CLOSE:
		printf("stop recoder...\r\n");
		break;
	default:
		break;
	}
}
