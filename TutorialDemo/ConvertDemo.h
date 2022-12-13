#pragma once
#include "common.h"
#include "SafeQueue.h"

class CAudioFrame
{
public:
	~CAudioFrame();

	bool Open(const char* szfile, int begin = 0, int end = 0);

	AVFrame* AudioFrame(bool& bstatus);

private:
	void Release();

	void OnRun();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;

	int64_t m_begin_pts = 0, m_end_pts = 0;
	int m_audio_idx = -1;

	SafeQueue<AVFrame*> m_frameQueueData;
	std::thread m_tRead;
	bool m_bRun = false;

	double m_timebase = 0.0;
};

class AEncode
{
public:
	bool Open(AVFormatContext* fmt_ctx, AVCodecID codec_id, int src_samples, AVChannelLayout src_ch_layout, AVSampleFormat src_fmt);
	
private:


};

class ConvertDemo
{
public:
	bool Save(const char* szOut, const char* szInput);

private:
	void OpenOutput(const char* szOut);

	void Release();

	void OnThreadFunc();

private:
	CAudioFrame audioFrame;
	AEncode		audioEncode;

	AVFormatContext* m_pFormatCtx = nullptr;


	std::thread m_thread;
};

