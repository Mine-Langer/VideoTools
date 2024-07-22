#pragma once
#include "Common.h"
#include <Audioclient.h>
#include <mmdeviceapi.h>
	
#pragma comment(lib, "Avrt.lib")

static int64_t getCurTimestamp();

// 录制音频相关回调接口
class IRecordEvent
{
public:
	virtual void AudioBufEvent(uint8_t* pBuf, int BufSize, int64_t pts) = 0;
};


class CAudioRecorder
{
public:
	CAudioRecorder(int CaptureType = SOUNDCARD);
	~CAudioRecorder();

	bool Init(int CapType);

	int get_nb_channels();		// 获取采样声道数（常用值：2,1）
	int get_nb_bits_sample();	// 获取采样时每一个采样点的存储比特数（常用值：16,8,24）
	int get_sample_rate();		// 获取采样率（常用值：44100,48000）
	int get_nb_samples();		// 获取采样时单声道一帧音频的采样点数量（常用值：44100->1024,48000->1056）

	
	void Start(IRecordEvent* pEvt);

	void Stop();

private:
	bool reInit();
	
	void Release();

	IMMDevice* GetDefaultDevice();
	bool AdjustFormat(WAVEFORMATEX* pwfx);

	void Work();

private:
	int m_CaptureType = 0;
	bool m_Inited = false;
	IMMDevice* m_pDevice = nullptr;
	WAVEFORMATEX* m_pWaveFmt = nullptr;
	IAudioClient* m_pAudioClient = nullptr;
	IAudioCaptureClient* m_pAudioCaptureClient = nullptr;
	HANDLE m_hTask = nullptr;

	IRecordEvent* m_pRecordEvent = nullptr;

	UINT m_nb_samples = 0; // 每一帧音频的采样点数量

	bool m_bRun = false;
	std::thread m_thread;
};

