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

	//bool Open(const char* szOutput);

	bool SetOutput(const char* szOutput, int vWidth, int vHeight, 
		AVChannelLayout chLayout, AVSampleFormat sampleFmt, int sampleRate, int bitRate);

	void SendFrame(AVFrame* frame, int nType);

	void Release();

	bool Start(IRemuxEvent* pEvt);

	void SetType(AVType type);

private:
	virtual bool VideoEvent(AVPacket* pkt) override;

	virtual bool AudioEvent(AVPacket* pkt, int64_t pts) override;

private:
	void OnWork();

	int WriteAudio(int64_t& idx);

	int WriteImage(int64_t& idx);

	int WriteVideo(int64_t& v_idx, int64_t& a_idx);


private:
	AVFormatContext* m_pFormatCtx = nullptr;
	IRemuxEvent* m_pEvent = nullptr;

	bool			m_bRun = false;
	std::thread		m_thread;
	AVType			m_avType;

	CAudioEncoder	m_audioEncoder;
	CVideoEncoder	m_videoEncoder;

	SafeQueue<AVFrame*> m_audioFrameQueue;
	SafeQueue<AVFrame*> m_videoFrameQueue;

	SafeQueue<int64_t> m_audioPtsQueue;
	SafeQueue<AVPacket*> m_audioPktQueue;
	SafeQueue<AVPacket*> m_videoPktQueue;
};

