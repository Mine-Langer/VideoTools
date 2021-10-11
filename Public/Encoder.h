#pragma once
#include "Common.h"

class IEncoderEvent
{
public:
	virtual void VideoEvent(AVPacket* vdata) = 0;
};

class CEncoder
{
};

