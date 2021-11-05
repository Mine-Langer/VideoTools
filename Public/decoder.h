#pragma once
#include "Common.h"


class STAudioBuffer
{
public:
	~STAudioBuffer() { if (buffer) { av_free(buffer); buffer = nullptr; } }

	uint8_t* buffer = nullptr;
	int size = 0;
	int64_t pts = 0;
	double dpts = 0;
	double duration = 0;
};


class IPlayEvent
{
public:
	virtual void UpdateDuration(double duration) = 0; // 更新总时长

	virtual void UpdatePlayPosition(double postion) = 0; // 更新播放位置
};

class IDecoderEvent
{
public:
	virtual void VideoEvent(AVFrame* vdata) = 0;

	virtual void AudioEvent(STAudioBuffer* adata) = 0;

	virtual void AudioEvent(AVFrame* adata) { }
};

class CDecoder
{
public:
	CDecoder();

	double GetTimebase();
	double GetRate();
	double GetDuration();

public:
	virtual bool SendPacket(AVPacket* pkt) = 0;
	virtual void OnDecodeFunction() = 0;
	virtual void Close() = 0;

protected:
	volatile bool m_bRun = false;
	IDecoderEvent* m_event = nullptr;
	std::thread m_decodeThread;
	SafeQueue<AVPacket*> m_packetQueue;

	AVCodecContext* m_CodecCtx = nullptr;

	double m_timebase = 0;
	double m_rate = 0;
	double m_duration = 0;
};

