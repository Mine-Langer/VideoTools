#pragma once
#include "Common.h"

class CFilterVideo
{
public:
	CFilterVideo();
	~CFilterVideo();

	bool Init(AVCodecContext* pCodecCtx, AVStream* pStream);

	void SetFilter(const char* szFilter);

	AVFrame* Convert(AVFrame* srcFrame);

private:
	AVFilterGraph* m_filterGraph = nullptr;
	AVFilterContext* m_bufferSinkCtx = nullptr;
	AVFilterContext* m_bufferSrcCtx = nullptr;

	std::string m_szFilter;
};

