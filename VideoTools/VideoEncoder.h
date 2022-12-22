#pragma once
#include "Common.h"

class IEncoderEvent
{
public:
	virtual bool VideoEvent(AVPacket* pkt) = 0;

	virtual bool AudioEvent(AVPacket* pkt, int64_t pts) = 0;
};

class CVideoEncoder
{
public:
	CVideoEncoder();
	~CVideoEncoder();

	bool Init(AVFormatContext* outFmtCtx, enum AVCodecID codec_id, int width, int height);

	void Start(IEncoderEvent* pEvt);

	void PushFrame(AVFrame* srcFrame);

	void Release();

	AVRational GetTimeBase();

	uint64_t GetIndex();

private:
	void OnWork();

private:
	IEncoderEvent* m_pEvent = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	AVStream* m_pStream = nullptr;
	SafeQueue<AVFrame*> m_videoDataQueue;
	uint64_t m_pts = 0;

	bool m_bRun = false;
	std::thread m_thread;
};

