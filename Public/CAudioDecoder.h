#pragma once
#include "Common.h"

class CAudioDecoder
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	bool Open(AVStream* pStream, enum AVCodecID codecId);

	void Start();

	void SendPacket(AVPacket* pkt);

private:
	void OnDecodeFunction();

private:
	bool m_bRun = false;
	AVCodecContext* AudioCodecCtx = nullptr;

	AVFrame* SrcFrame = nullptr;

	
	SafeQueue<AVPacket*> AudioPacketQueue;

	std::thread m_decodeThread;
};

