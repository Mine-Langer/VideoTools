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

private:
	void OnDecodeFunction();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;

	IVideoEvent* m_pEvent = nullptr;

	int m_videoIndex = -1;

	std::thread m_thread;
};



