#pragma once
#include "Remultiplexer.h"

class CVideoEncoder
{
public:
	CVideoEncoder();
	~CVideoEncoder();

	AVCodecContext* Init(enum AVCodecID codec_id, int width, int height);

	void Start(IRemuxEvent* pEvt);

	void Close();

	void SendFrame(AVFrame* srcFrame);

	void SetStreams(AVStream* pStream);

protected:
	void Work();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	SwsContext*		m_pSwsCtx = nullptr;
	AVStream*		m_pStream = nullptr;

	IRemuxEvent* m_pRemuxEvt = nullptr;

	SafeQueue<AVFrame*> m_videoDataQueue;

	uint64_t m_pts = 0;

	bool m_bRun = false;
	std::thread m_thread;
};

