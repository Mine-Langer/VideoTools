#pragma once

#include "Common.h"

class CAVSync
{
public:
	void SetAudioClock(double pts) // 设置当前音频主时钟
	{
		AudioClock = pts;
	}

	void SetShowTime()
	{
		int64_t t = av_gettime_relative();
		ShowTime = av_gettime_relative() / AV_TIME_BASE * 1.0;
	}

	int64_t CalcDelay(double vpts)
	{
		if (ShowTime == 0)
			SetShowTime();

		int i64_delay = 0;
		double diff = vpts - AudioClock;
		double delay = vpts - LastVideoClock;
		int series = std::to_string(static_cast<int64_t>(diff * AV_TIME_BASE)).size();

		LastVideoClock = vpts;

		if (diff > AVSYNC_DYNAMIC_THRESHOLD || diff < -AVSYNC_DYNAMIC_THRESHOLD) // 表示视频帧在前
			diff = diff * (std::pow(1.0 + AVSYNC_DYNAMIC_COEFFICIENT, series) - 1.0);

		if (delay > 0 && (delay + diff) > 0)
		{
			delay += diff;
			double elapseTime = av_gettime_relative() / AV_TIME_BASE * 1.0 - ShowTime;
			i64_delay = (delay - elapseTime) * AV_TIME_BASE;
		}
		return i64_delay;
	}

private:
	volatile double AudioClock = 0;
	volatile double LastVideoClock = 0;
	volatile double ShowTime = 0;
};