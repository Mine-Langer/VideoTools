#pragma once
#include "Demultiplexer.h"

class CVideoDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();

	bool Open(const std::string& szFile);
	bool Open(CDemultiplexer& demux);

	void Start(IDecoderEvent* pEvt);

	void Close();

	bool SendPacket(AVPacket* pkt);

	bool InitSwsContext(int w=-1, int h=-1, enum AVPixelFormat pix_fmt=AV_PIX_FMT_NONE);

	double GetTimebase() const;

protected:
	void Work();


private:
	AVCodecContext* m_pCodecCtx = nullptr;
	SwsContext*		m_pSwsCtx = nullptr;

	IDecoderEvent* m_DecoderEvt = nullptr;

	SafeQueue<AVPacket*> m_pktQueue;

	double m_timebase;
	double m_duration;
	double m_rate;

	int m_swsWidth = -1;
	int m_swsHeight = -1;
	enum AVPixelFormat m_swsPixFmt = AV_PIX_FMT_NONE;

	bool m_bRun = false;
	std::thread m_thread;
};

