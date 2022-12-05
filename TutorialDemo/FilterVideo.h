#pragma once
#include "common.h"

class FilterVideo
{
public:
	FilterVideo();
	~FilterVideo();

	bool Run(const char* szInput, const char* szOutput, int x, int y, int size, const char* szText);

private:
	bool OpenInput(const char* szInput);

	bool OpenOutput(const char* szOutput);

	bool OpenFilter(const char* szFilterDesc);

	void OnDemux();

private:
	AVFormatContext* m_fmt_ctx_in = nullptr;
	AVCodecContext* m_codec_ctx_in = nullptr;

	AVFormatContext* m_fmt_ctx_out = nullptr;
	AVCodecContext* m_codec_ctx_out = nullptr;

	AVFilterGraph* m_pFilterGraph = nullptr;
	AVFilterContext* m_pFilterCtxSrc = nullptr;
	AVFilterContext* m_pFilterCtxSink = nullptr;
	AVFilterContext* m_pFilterCtxText = nullptr;

	int m_videoIndex = -1;
};

