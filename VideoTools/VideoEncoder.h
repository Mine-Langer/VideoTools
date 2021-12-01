#pragma once
#include "Common.h"

class CVideoEncoder
{
public:
	CVideoEncoder();
	~CVideoEncoder();

	bool Init(AVFormatContext* outFmtCtx, enum AVCodecID codec_id, int width, int height);

	AVPacket* Encode(AVFrame* srcFrame);

	void Release();

	void Close();

	AVRational GetTimeBase();

private:
	void OnEncodeFunction();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	AVStream* m_pStream = nullptr;

	bool m_bRun = false;
};

