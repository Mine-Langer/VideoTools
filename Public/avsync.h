#pragma once

#include "Common.h"

class CAVSync
{
public:
	void SetAudioClock(int64_t pts) // 设置当前音频主时钟
	{
		AudioClock = pts;
	}



private:
	volatile int64_t AudioClock = 0;
	volatile int64_t VideoClock = 0;
	volatile double ShowTime = 0;
};