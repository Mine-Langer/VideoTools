#pragma once
#include "decoder.h"

class CVideoDecoder :public CDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();

	bool Open(AVStream* pStream);

	void Start(IDecoderEvent* evt);

	bool SetConfig(int width, int height, AVPixelFormat iformat, int iflags);

	bool SendPacket(AVPacket* pkt) override;
	void Close() override;
protected:

private:
	void OnDecodeFunction() override;

private:
	bool m_bRun = false;

	AVCodecContext* VideoCodecCtx = nullptr;
	AVFrame* SrcFrame = nullptr;
	AVFrame* DstFrame = nullptr;

	SwsContext* SwsCtx = nullptr;

};

