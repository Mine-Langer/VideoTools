#pragma once
#include "AudioEncoder.h"
#include "VideoEncoder.h"
#include "AudioDecoder.h"
#include "VideoEncoder.h"
/*
* ÖØ·â×°¸´ÓÃÆ÷
*/
class CRemultiplexer
{
public:
	CRemultiplexer();
	~CRemultiplexer();

	bool SetOutput(const char* szOutput, int vWidth, int vHeight, AVChannelLayout chLayout, AVSampleFormat sampleFmt, int sampleRate);

	void SendFrame(AVFrame* frame, int nType);

	void Release();

	bool Start();

private:
	void OnWork();

private:
	AVFormatContext* m_pFormatCtx = nullptr;

	bool			m_bRun = false;
	std::thread		m_thread;

	CAudioEncoder	m_audioEncoder;
	CVideoEncoder	m_videoEncoder;

	CAudioDecoder	m_audioDecoder;
	CVideoDecoder	m_videoDecoder;

	SafeQueue<AVFrame*> m_audioFrameQueue;
	SafeQueue<AVFrame*> m_videoFrameQueue;
};

