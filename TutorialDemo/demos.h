#pragma once
#include "Common.h"

class CScaleImage
{
public:
	bool Scale(const char* szFile, const char* nSize);

private:
	void fill_yuv_image(int frame_index);

private:

	int m_srcWidth = 320;
	int m_srcHeight = 240;
	AVPixelFormat m_srcPixelFmt = AV_PIX_FMT_YUV420P;
	uint8_t *m_srcData[4], *m_dstData[4];
	int m_srcLinesize[4], m_dstLineSize[4];
	int m_dstBufSize = 0;
};

