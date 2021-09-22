#pragma once
#include "Common.h"

class CVideoDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();

	bool Open(AVStream* pStream, enum AVCodecID codecId);

	void Start();

	void SendPacket(AVPacket* pkt);

private:
	void OnDecodeFunction();

private:
	bool m_bRun = false;

	AVCodecContext* VideoCodecCtx = nullptr;
	AVFrame* SrcFrame = nullptr;


	std::thread m_decodeThread;

	SafeQueue<AVPacket*> VideoPacketQueue;
};

