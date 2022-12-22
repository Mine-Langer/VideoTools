#pragma once
#include "AudioEncoder.h"
#include "VideoEncoder.h"
#include "AudioDecoder.h"
#include "VideoEncoder.h"

class IRemuxEvent
{
public:
	virtual void RemuxEvent(int nType) = 0;
};
/*
* ÖØ·â×°¸´ÓÃÆ÷
*/
class CRemultiplexer: public IEncoderEvent
{
public:
	CRemultiplexer();
	~CRemultiplexer();

	bool SetOutput(const char* szOutput, int vWidth, int vHeight, AVChannelLayout chLayout, AVSampleFormat sampleFmt, int sampleRate);

	void SendFrame(AVFrame* frame, int nType);

	void Release();

	bool Start(IRemuxEvent* pEvt);

private:
	virtual bool VideoEvent(AVPacket* pkt) override;

	virtual bool AudioEvent(AVPacket* pkt, int64_t pts) override;

private:
	void OnWork();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	IRemuxEvent* m_pEvent = nullptr;

	bool			m_bRun = false;
	std::thread		m_thread;

	CAudioEncoder	m_audioEncoder;
	CVideoEncoder	m_videoEncoder;

	SafeQueue<AVFrame*> m_audioFrameQueue;
	SafeQueue<AVFrame*> m_videoFrameQueue;

	SafeQueue<int64_t> m_audioPtsQueue;
	SafeQueue<AVPacket*> m_audioPktQueue;
	SafeQueue<AVPacket*> m_videoPktQueue;
};

