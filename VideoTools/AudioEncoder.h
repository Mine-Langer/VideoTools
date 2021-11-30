#pragma once
#include "Common.h"

class CAudioEncoder
{
public:
	CAudioEncoder();
	~CAudioEncoder();

	bool InitAudio(AVFormatContext* formatCtx, AVCodecID codecId);

	void Release();

	AVPacket* Encode(AVFrame* srcFrame);

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	AVStream* m_pStream = nullptr;

	int m_nbSamples = 0;
};

