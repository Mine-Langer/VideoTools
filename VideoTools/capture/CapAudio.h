#pragma once
#include "../Common.h"

extern char* dup_wchar_to_utf8(const wchar_t* w);

class CCapAudio
{
public:
	CCapAudio();
	~CCapAudio();

	bool Init(enum AVSampleFormat sample_fmt, int nChannels, int channel_layout, int sample_rate, int frame_size);

	bool Start(IAudioEvent* pEvt);

	void Release();

private:
	void OnDecodeFunction(); // 解码线程
	void OnConvertFunction(); // 转码线程

private:
	wchar_t* GetMicrophoneName();
	void PushFrame(AVFrame* frame);
	void ReadFifoFrame();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	AVAudioFifo* m_audioFifo = nullptr;

	IAudioEvent* m_pEvent = nullptr;
	SwrContext* m_swrCtx = nullptr;

	enum AVState m_state = NotStarted;

	int m_audioIndex = -1;
	int m_frameSize = 0;
	int m_outSampleRate = 0;
	int m_outChannels = 0;
	int m_outChannelLayout = 0;
	int m_nbSamples = 0;
	int m_frameIndex = 0;
	AVSampleFormat m_outSampleFmt = AV_SAMPLE_FMT_NONE;

	std::queue<AVFrame*> m_audioQueue;

	std::thread m_thread;
	std::thread m_tConvert;
};

