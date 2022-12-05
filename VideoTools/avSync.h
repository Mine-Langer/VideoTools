#pragma once
#include "Common.h"

class AVSync
{
public:
	void SetAudioClock(double _pts)
	{
		audioClock = _pts;
	}

	void SetVideoShowTime()
	{
		videoShowStartTime = av_gettime_relative() / (AV_TIME_BASE * 1.0);
	}

	int64_t CalcDelay(double _pts)
	{
		int64_t i64_delay = 0;
		double elapsed_time = 0.0;

		if (videoShowStartTime == 0)
			SetVideoShowTime();

		double diff = _pts - audioClock;
		double delay = _pts - lastVideoPts;
		int serial = std::to_string(static_cast<int64_t>(diff * AV_TIME_BASE)).size();

		lastVideoPts = _pts;

		diff = diff * (std::pow(1.0 + AVSYNC_DYNAMIC_COEFFICIENT, serial) - 1.0);

		if (diff > 0.0 && (delay + diff) > 0.0)
		{
			delay += diff;
			elapsed_time = av_gettime_relative() / (AV_TIME_BASE * 1.0) - videoShowStartTime;
			i64_delay = static_cast<int64_t>((delay - elapsed_time) * AV_TIME_BASE);
		}
		else
		{
			i64_delay = -1;
		}

		return i64_delay;
	}

private:
	volatile double audioClock = 0.0;
	volatile double lastVideoPts = 0.0;
	volatile double videoShowStartTime = 0.0;
};