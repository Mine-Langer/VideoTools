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
	HWAVEIN hWaveIn; // 输入设备
	HWAVEOUT hWaveOut; // 输出设备
	WAVEFORMATEX waveFormat = { 0 }; // 音频流格式
	WAVEHDR whdr[2]; // 输入音频头
	WAVEHDR whdr_o; // 输出音频头

	BYTE* file = nullptr;
	DWORD hasRecorded = 0;
	BOOL IsStop = FALSE;
};

