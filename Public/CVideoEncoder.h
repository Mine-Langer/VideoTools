#pragma once
#include "Encoder.h"

class CVideoEncoder
{
public:
	CVideoEncoder();
	~CVideoEncoder();

	bool InitConfig(AVFormatContext* outputFmtCtx, int width, int height);

	void Start(IEncoderEvent* evt);

	void Encode(AVFrame* frame);

private:
	AVCodecContext* VideoCodecCtx = nullptr;
	AVStream* VideoStream = nullptr;

	IEncoderEvent* event = nullptr;
};

