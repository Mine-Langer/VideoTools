#include "recoder.h"

CRecoder::CRecoder()
{

}

CRecoder::~CRecoder()
{

}

void CRecoder::SetWaveFormat(int nSamplesRate, int nBit, int nChannel)
{
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = nChannel;
	waveFormat.nSamplesPerSec = nSamplesRate;
	waveFormat.nAvgBytesPerSec = nSamplesRate * nChannel * nBit / 8;
	waveFormat.nBlockAlign = nChannel * nBit / 8;
	waveFormat.wBitsPerSample = nBit;
	waveFormat.cbSize = 0;
}

bool CRecoder::Init()
{
	if (GetMicrophone())
	{
		SetWaveFormat(44100, 16, 2);
		
		// ��ʼ���豸������
		for (int i = 0; i < 2; i++)
		{
			whdr[i].lpData = (LPSTR)malloc(4096);
			memset(whdr[i].lpData, 0, sizeof(4096));
			whdr[i].dwBufferLength = 4096;
			whdr[i].dwBytesRecorded = 0;
			whdr[i].dwUser = 0;
			whdr[i].dwFlags = 0;
			whdr[i].dwLoops = 0;
		}
		return true;
	}
	return false;
}

bool CRecoder::Start()
{
	try
	{
		MMRESULT mr = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD_PTR)RecoderFunction, (DWORD_PTR)this, CALLBACK_FUNCTION);
		if (mr != MMSYSERR_NOERROR)
		{
			return false;
		}

		waveInPrepareHeader(hWaveIn, &whdr[0], sizeof(WAVEHDR));	//�������ݿ�
		waveInPrepareHeader(hWaveIn, &whdr[1], sizeof(WAVEHDR));

		//���𻺴�
		waveInAddBuffer(hWaveIn, &whdr[0], sizeof(WAVEHDR));		//ѹ�뻺����
		waveInAddBuffer(hWaveIn, &whdr[1], sizeof(WAVEHDR));

		//����¼����ʼ��Ϣ
		waveInStart(hWaveIn);
		IsStop = FALSE;

		printf("��ʼ¼��\n");
		return true;
	}
	catch (...)
	{
		printf("����ʧ��\n");
		return false;
	}
}

void CRecoder::Stop()
{
	try
	{
		IsStop = TRUE;
		waveInStop(hWaveIn);
		waveInReset(hWaveIn);

		waveInUnprepareHeader(hWaveIn, &whdr[0], sizeof(WAVEHDR));
		waveInUnprepareHeader(hWaveIn, &whdr[1], sizeof(WAVEHDR));

		waveInClose(hWaveIn);
		printf("¼��ֹͣ��\r\n");
	}
	catch (...)
	{
		printf("ֹͣʧ�ܣ�\r\n");
	}
}

void CRecoder::Play()
{
	try
	{
		MMRESULT mr = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD_PTR)PlayFunction, (DWORD_PTR)this, CALLBACK_FUNCTION);
		if (mr != MMSYSERR_NOERROR)
			return;

		while (true)
		{
			for (int i = 0; i < 2; i++)
			{
				if (whdr[i].dwUser != 1)
				{
					char szBuffer[4096] = { 0 }; // ��Ƶ��
					
					memcpy(whdr[i].lpData, szBuffer, 4096);
					whdr[i].dwUser = 1;

					waveOutPrepareHeader(hWaveOut, &whdr[i], sizeof(WAVEHDR));
					waveOutWrite(hWaveOut, &whdr[i], sizeof(WAVEHDR));
				}
			}
		}
	}
	catch (...)
	{

	}
}

bool CRecoder::GetMicrophone()
{
	// ��ȡ��Ƶ�豸����
	int nCount = waveInGetNumDevs();
	if (nCount == 0)
	{
		return false;
	}

	// ��ȡ�豸��Ϣ
	WAVEINCAPS waveCaps = { 0 };
	MMRESULT mr = waveInGetDevCaps(0, &waveCaps, sizeof(WAVEINCAPS));
	if (mr != MMSYSERR_NOERROR)
		return false;

	return true;
}

void CALLBACK CRecoder::RecoderFunction(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInst, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	CRecoder* pThis = (CRecoder*)dwInst;
	// ��ȡ��Ƶͷ
	PWAVEHDR pwhdr = (PWAVEHDR)dwParam1;
	char buf[4096] = { 0 };
	// ������Ϣ
	switch (uMsg)
	{
	case WIM_OPEN:
		printf("Open Device succeed!\r\n");
		break;
	case WIM_DATA:
	{
		printf("Buffer has Complete!\r\n");
		LPWAVEHDR pwhdr = (LPWAVEHDR)dwParam1;
		memcpy(buf, pwhdr->lpData, 4096);

		if (!pThis->IsStop)
		{
			memset(pwhdr->lpData, 0, 4096);
			pwhdr->dwBytesRecorded = 0;

			waveInAddBuffer(hwi, (PWAVEHDR)dwParam1, sizeof(WAVEHDR));
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

void CALLBACK CRecoder::PlayFunction(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInst, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	LPWAVEHDR phdr = (LPWAVEHDR)dwParam1;
	switch (uMsg)
	{
	case WOM_OPEN:
		printf("�����豸������\r\n");
		break;
	case WOM_DONE:
		// �������� ʹ���������¶�ȡ����
		phdr->dwUser = 0;
		break;
	case WOM_CLOSE:
		printf("�����豸�رգ�\r\n");
		break;
	}
}
