#pragma once
#define AVSYNC_THRESHOLD 0.003
#define AVSYNC_COEFFICIENT (std::pow(1.10, 1.0 / 6.0) - 1)

class CAVSync
{
public:
	void SetAudioClock(double _pts)
	{
		audioClock = _pts;
	}

	void SetVideoShowTime()
	{
		videoShowStartTime = av_gettime_relative() / AV_TIME_BASE * 1.0;
	}

	int64_t CalcDelay(double pts)
	{
		if (videoShowStartTime == 0.0)
			SetVideoShowTime();
		
		int64_t i64Delay = 0;
		double diff = pts - audioClock; 	// 当前视频帧时钟与音频帧时钟的理论差值
		double delay = pts - lastVideoPts;	// 当前视频帧与上一帧的差值

		lastVideoPts = pts;

		diff = diff * (std::pow(1.0 + AVSYNC_COEFFICIENT, 6) - 1.0);
		if (delay > 0 && (delay + diff) > 0)
		{
			delay += diff;
			double elapsedTime = av_gettime_relative() / AV_TIME_BASE * 1.0 - videoShowStartTime;
			i64Delay = static_cast<int64_t>((delay - elapsedTime) * AV_TIME_BASE);
		}
		else
			i64Delay = -1;

		return i64Delay;
	}

private:
	volatile double audioClock = 0.0;		//主时钟 音频时钟
	volatile double lastVideoPts = 0.0;		//上一帧视频PTS
	volatile double videoShowStartTime = 0.0;// 视频帧显示周期的起始时间
};