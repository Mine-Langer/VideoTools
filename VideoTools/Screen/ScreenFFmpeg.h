#pragma once
#include "ScreenRecoder.h"
#include "../Common.h"

class CScreenFFmpeg : public CScreenRecoder
{
public:
	CScreenFFmpeg();
	~CScreenFFmpeg();

protected:
	virtual bool GetFrame(uint8_t* buffer, int& nSize, int64_t& timestamp) override;

	void Init();

	void Release();

private:
	AVFormatContext* m_FmtCtx = nullptr;
	AVCodecContext* m_CodecCtx = nullptr;
	AVStream* m_Stream = nullptr;
	int       m_Index = -1;
	AVPacket* m_Pkt = nullptr;
	AVFrame* srcFrame = nullptr;

	AVFrame* dstFrame = nullptr;
	uint8_t* dstFrame_buff = nullptr;

	SwsContext* m_VideoSwsCtx = nullptr;
	int dstDataSize = 0;
	const char* m_Capture = "unknown";
};

