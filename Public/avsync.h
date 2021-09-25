#pragma once

#include "Common.h"

class CAVSync
{
public:
	void SetAudioClock(int64_t pts) // ���õ�ǰ��Ƶ��ʱ��
	{
		AudioClock = pts;
	}



private:
	volatile int64_t AudioClock = 0;
	volatile int64_t VideoClock = 0;
	volatile double ShowTime = 0;
};