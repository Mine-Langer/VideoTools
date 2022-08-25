#include "demos.h"

bool CScaleImage::Scale(const char* szFile, const char* szSize)
{
	int iRet = 0;
	int width, height;
	iRet = av_parse_video_size(&width, &height, szSize);
	if (iRet < 0)
	{
		char szError[_MAX_PATH] = { 0 };
		av_make_error_string(szError, _MAX_PATH, iRet);
		fprintf(stderr, "Invalid size '%s', must be in the from WxH or a valid size abbreviation\r\n", szSize);
		fprintf(stderr, "error:%s. \r\n", szError);
	}

	FILE* hFile = NULL;
	if (0 > fopen_s(&hFile, szFile, "wb"))
		return false;

	AVPixelFormat dstPixFmt = AV_PIX_FMT_BGR24;

	SwsContext* sws_ctx = sws_getContext(m_srcWidth, m_srcHeight, m_srcPixelFmt,
		width, height, dstPixFmt, SWS_BICUBIC, NULL, NULL, NULL);
	if (!sws_ctx)
		return false;

	if (0 > (iRet = av_image_alloc(m_srcData, m_srcLinesize, m_srcWidth, m_srcHeight, m_srcPixelFmt, 16)))
		return false;

	if (0 > (iRet = av_image_alloc(m_dstData, m_dstLineSize, width, height, dstPixFmt, 1)))
		return false;

	m_dstBufSize = iRet;

	for (int i = 0; i < 100; i++)
	{
		fill_yuv_image(i);

		sws_scale(sws_ctx, m_srcData, m_srcLinesize, 0, m_srcHeight, m_dstData, m_dstLineSize);

		fwrite(m_dstData[0], 1, m_dstBufSize, hFile);
	}

	fclose(hFile);

	return false;
}

void CScaleImage::fill_yuv_image(int frame_index)
{
}
