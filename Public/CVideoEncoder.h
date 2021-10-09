#pragma once
#include "Common.h"

class CVideoEncoder
{
public:
	CVideoEncoder();
	~CVideoEncoder();

	bool InitConfig(AVFormatContext* outputFmtCtx);

private:
	AVCodecContext* VideoCodecCtx = nullptr;
};

