#pragma once
#include "Common.h"

class IDecoderEvent
{
public:
	virtual bool VideoEvent(AVFrame* vdata) = 0;
	
	virtual bool AudioEvent(AVFrame* adata) = 0;
};

class CVideoDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();

	bool Open(AVStream* pStream);

	void Start(IDecoderEvent* pEvent);

	void Close();

	void SetConfig(SDL_Rect& rect, int dstWidth, int dstHeight);

	void PushPacket(AVPacket* pkt);

	AVFrame* ConvertFrame(AVFrame* frame);

	double Timebase() { return m_timebase; }

private:
	void OnDecodeThread();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	struct SwsContext* m_pSwsCtx = nullptr;
	IDecoderEvent* m_pEvent = nullptr;

	eAVStatus m_avStatus = eStop;
	std::thread m_decodeThread;

	int m_swsWidth = 0;
	int m_swsHeight = 0;
	AVPixelFormat m_swsFormat = AV_PIX_FMT_NONE;

	double m_timebase = 0.0f;
	double m_duration = 0.0f;

	SafeQueue<AVPacket*> m_vPktQueue;
};

