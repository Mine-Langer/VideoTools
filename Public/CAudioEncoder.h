#pragma once
#include "Encoder.h"
#include "CAudioDecoder.h"

class CAudioEncoder
{
public:
	CAudioEncoder();
	~CAudioEncoder();

	bool InitAudio(AVFormatContext* formatCtx, AVCodecID codecId, CAudioDecoder* audioDecoder);

	void Start(IEncoderEvent* pEvt);

	void PushFrame(AVFrame* frame);

	void Release();

private:
	void OnEncodeThread();

	void ConvertAudioBuffer();

	void GetConvertBuffer();

	void EncodeFrame(AVFrame* frame);

private:
	bool m_bRun = false;
	AVCodecContext* AudioCodecCtx = nullptr;
	AVStream* AudioStream = nullptr;

	SwrContext* SwrCtx = nullptr;
	AVAudioFifo* AudioFIFO = nullptr;

	IEncoderEvent* m_event = nullptr;

	SafeQueue<AVFrame*> m_audioFrameQueue;
	std::thread m_encodeThread;
	int m_pts = 0;
	int m_nbSamples = 0;
};

