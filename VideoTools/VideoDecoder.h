#pragma once
#include "Common.h"

class CVideoDecoder
{
public:
	CVideoDecoder();
	virtual ~CVideoDecoder();

	virtual bool Open(const char* szInput) = 0;

	virtual void Start() = 0;

	virtual void Stop() = 0;

	virtual void Release() = 0;

	virtual void SetVideoInfo(int x, int y, int width, int height) = 0;

	virtual void VideoEvent(AVFrame* frame) = 0;

protected:
	AVFormatContext* m_pFormatCtx = nullptr;
};

