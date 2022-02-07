#pragma once
#include "VideoDecoder.h"

class CAudioDecoder
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	bool Open(AVStream* pStream);

	void Start(IDecoderEvent* pEvent);

	void Close();

	void PushPacket(AVPacket* pkt);

	AVFrame* ConvertFrame(AVFrame* frame);

	void InitAudioSpec(SDL_AudioSpec& audioSpec);

	double Timebase() { return m_timebase; }

private:
	void OnDecodeThread();

	bool SetConfig();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	IDecoderEvent* m_pEvent = nullptr;
	SwrContext* m_pSwrCtx = nullptr;

	eAVStatus m_status = eStop;


	double m_timebase = 0.0f;
	double m_duration = 0.0f;

	int64_t m_channelLayout = 0;
	AVSampleFormat m_sampleFmt = AV_SAMPLE_FMT_NONE;
	int m_sampleRate = 0;
	int64_t m_nbSamples = 0;

	SafeQueue<AVPacket*> m_audioQueue;

	std::thread m_decodeThread;
};

