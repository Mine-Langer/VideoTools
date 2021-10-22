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

	void Release();

private:
	void OnEncodeThread();

private:
	bool m_bRun = false;
	AVCodecContext* AudioCodecCtx = nullptr;
	AVStream* AudioStream = nullptr;

	SwrContext* SwrCtx = nullptr;
	AVAudioFifo* AudioFIFO = nullptr;

	IEncoderEvent* m_event = nullptr;

	std::thread m_encodeThread;
};

