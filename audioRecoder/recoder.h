#pragma once
#include <iostream>
#include <windows.h>
#include <mmsystem.h>

#pragma comment (lib, "winmm.lib")

#define BUFFER_LEN  1024*10000

class CRecoder
{
public:
	CRecoder();
	~CRecoder();

	bool Init();
	
	void Start();

private:
	void SetConfig(int nSamples, int nBit, int nChannel);

	bool GetMicrophone();

	static void CALLBACK RecoderFunction(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInst, DWORD_PTR dwParam1, DWORD_PTR dwParam2);


private:
	HWAVEIN hWaveIn; // �����豸
	HWAVEOUT hWaveOut; // ����豸
	WAVEFORMATEX waveFormat = { 0 }; // ��Ƶ����ʽ
	WAVEHDR whdr[2]; // ������Ƶͷ
	WAVEHDR whdr_o; // �����Ƶͷ

	BYTE* file = nullptr;
	DWORD hasRecorded = 0;
	BOOL IsStop = FALSE;
};

