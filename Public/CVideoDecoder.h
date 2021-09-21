#pragma once
#include "Common.h"

class CVideoDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();

	bool Open(AVStream* pStream, enum AVCodecID codecId);

	void Start();

private:
	AVCodecContext* VideoCodecCtx = nullptr;
};

