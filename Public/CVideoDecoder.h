#pragma once
#include "decoder.h"

class CVideoDecoder :public CDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();

	bool Open(AVStream* pStream);

	void Start(IDecoderEvent* evt);

	bool SetConfig(int width = -1, int height = -1, AVPixelFormat iformat = AV_PIX_FMT_NONE, int iflags = SWS_BICUBIC);
	void GetParameter(int& width, int& height, AVPixelFormat& iformat);

	bool SendPacket(AVPacket* pkt) override;
	void Close() override;
protected:

private:
	void OnDecodeFunction() override;

private:
	bool m_bRun = false;
	int VideoWidth = 0, VideoHeight = 0;
	AVPixelFormat VideoFormat = AV_PIX_FMT_NONE;
	AVCodecContext* VideoCodecCtx = nullptr;
	AVFrame* SrcFrame = nullptr;
	AVFrame* DstFrame = nullptr;

	SwsContext* SwsCtx = nullptr;
	int64_t m_nFrameCount = 0;
};

