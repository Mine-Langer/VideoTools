#pragma once
#include "Common.h"

class IDecoderEvent
{
public:
	virtual void VideoEvent(AVFrame* vdata) = 0;

	virtual void AudioEvent(AVFrame* adata) = 0;
};

class CDecoder
{
public:
	CDecoder();

	double GetTimebase();
	double GetRate();
	double GetDuration();

public:
	virtual bool SendPacket(AVPacket* pkt) = 0;
	virtual void OnDecodeFunction() = 0;
	virtual void Close() = 0;

protected:
	volatile bool m_bRun = false;
	IDecoderEvent* m_event = nullptr;
	std::thread m_decodeThread;
	SafeQueue<AVPacket*> m_packetQueue;

	AVCodecContext* m_CodecCtx = nullptr;

	double m_timebase = 0;
	double m_rate = 0;
	double m_duration = 0;
};

