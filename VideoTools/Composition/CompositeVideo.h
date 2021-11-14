#pragma once
#include "../Common.h"


class CompositeVideo
{
public:
	CompositeVideo();
	~CompositeVideo();

	bool Open(const char* szFile);

	void Start(IVideoEvent* pEvt);

	void Release();

	bool SetFrameSize(int width, int height, enum AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P);
	AVFrame* ConvertFrame(AVFrame* frame);

private:
	void OnDecodeFunction();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	SwsContext* m_pSwsCtx = nullptr;

	IVideoEvent* m_pEvent = nullptr;

	int m_videoIndex = -1;
	int m_swsWidth = 0;
	int m_swsHeight = 0;
	AVPixelFormat m_swsPixfmt = AV_PIX_FMT_NONE;

	std::thread m_thread;
};



