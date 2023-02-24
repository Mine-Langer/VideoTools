#pragma once
#include "common.h"

class ImageConvert
{
public:
	ImageConvert();
	~ImageConvert();

	bool Open(const char* szFile);

	bool SetOutput();

	void Work();

private:
	AVFormatContext* InputFormatCtx = nullptr;
	AVCodecContext* InputCodecCtx = nullptr;

	AVFormatContext* OutputFormatCtx = nullptr;
	AVCodecContext* OutputCodecCtx = nullptr;

	int m_srcWidth, m_srcHeight;
};

