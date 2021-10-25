// audioRecoder.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// https://blog.csdn.net/Ephemeroptera/article/details/100190973

#include <iostream>
#include <windows.h>
#include <mmsystem.h>

#pragma comment (lib, "winmm.lib")

#define BUFFER_LEN  1024*10000

void CALLBACK RecoderFunction(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInst, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

int main()
{
    std::cout << "Hello World!\n";
    HWAVEIN hWaveIn; // 输入设备
    HWAVEOUT hWaveOut; // 输出设备
    WAVEFORMATEX waveFormat = { 0 }; // 音频流格式
    WAVEHDR whdr_i1, whdr_i2; // 输入音频头
    WAVEHDR whdr_o; // 输出音频头

    // 设备数量
    int count = waveInGetNumDevs();

    // 设备名称
    WAVEINCAPS waveInCaps = { 0 };
    MMRESULT mmResult = waveInGetDevCaps(0, &waveInCaps, sizeof(WAVEINCAPS));

	// 设置音频流格式
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
    waveFormat.nSamplesPerSec = 44100;  // 采样率
    waveFormat.wBitsPerSample = 16;     // 采样精度
    waveFormat.nChannels = 2;           // 声道数
    waveFormat.wFormatTag = WAVE_FORMAT_PCM; // 音频格式
    waveFormat.nBlockAlign = (waveFormat.wBitsPerSample * waveFormat.nChannels) / 8; // 块对齐
    waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec; // 传输速率

    // 分配内存
    BYTE* pBuffer1 = new BYTE[BUFFER_LEN]();
    BYTE* pBuffer2 = new BYTE[BUFFER_LEN]();

    // 设置音频头
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

    // 开启录音
    mmResult = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD_PTR)RecoderFunction, 0, CALLBACK_FUNCTION);

    waveInPrepareHeader(hWaveIn, &whdr_i1, sizeof(WAVEHDR));
    waveInPrepareHeader(hWaveIn, &whdr_i2, sizeof(WAVEHDR));
}

void CALLBACK RecoderFunction(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInst, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{

}