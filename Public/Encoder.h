#pragma once
#include "Common.h"

class IEncoderEvent
{
public:
	virtual void VideoEvent(AVPacket* vdata) = 0;

	virtual void AudioEvent(AVPacket* adata) = 0;
};

class CEncoder
{
};

