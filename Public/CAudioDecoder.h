#pragma once
#include "Common.h"

class CAudioDecoder
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	bool Open(AVStream* pStream, enum AVCodecID codecId);

	void Start();

private:
	void OnDecodeFunction();

private:
	AVCodecContext* AudioCodecCtx = nullptr;
	
	std::thread m_decodeThread;
};

