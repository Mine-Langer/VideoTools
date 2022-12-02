#include "FilterVideo.h"

CFilterVideo::CFilterVideo()
{

}

CFilterVideo::~CFilterVideo()
{

}

bool CFilterVideo::Init(AVCodecContext* pCodecCtx, AVStream* pStream)
{
	char szArgs[512] = { 0 };
	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();
	AVRational timebase = pStream->time_base;
	AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

	sprintf_s(szArgs, sizeof(szArgs), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		pCodecCtx->width/2*2, pCodecCtx->height/2*2, pCodecCtx->pix_fmt, timebase.num, timebase.den,
		pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);

	m_filterGraph = avfilter_graph_alloc();
	if (0 > avfilter_graph_create_filter(&m_bufferSrcCtx, buffersrc, "in", szArgs, nullptr, m_filterGraph))
		return false;

	AVBufferSinkParams* bufSinkParam = av_buffersink_params_alloc();
	bufSinkParam->pixel_fmts = pix_fmts;

	if (0 > avfilter_graph_create_filter(&m_bufferSinkCtx, buffersink, "out", nullptr, bufSinkParam, m_filterGraph))
	{
		av_free(bufSinkParam);
		return false;
	}
	av_free(bufSinkParam);


	outputs->name = av_strdup("in");
	outputs->filter_ctx = m_bufferSrcCtx;
	outputs->pad_idx = 0;
	outputs->next = nullptr;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = m_bufferSinkCtx;
	inputs->pad_idx = 0;
	inputs->next = nullptr;

	if (0 > avfilter_graph_parse_ptr(m_filterGraph, m_szFilter.c_str(), &inputs, &outputs, nullptr))
		return false;

	if (0 > avfilter_graph_config(m_filterGraph, nullptr))
		return false;

	char* temp = avfilter_graph_dump(m_filterGraph, nullptr);

	return true;
}

void CFilterVideo::SetFilter(const char* szFilter)
{
	m_szFilter = szFilter;
}

AVFrame* CFilterVideo::Convert(AVFrame* srcFrame)
{
	AVFrame* dstFrame = av_frame_alloc();
	if (0 > av_buffersrc_add_frame(m_bufferSrcCtx, srcFrame))
	{
		av_frame_free(&dstFrame);
		return nullptr;
	}
	
	if (0 > av_buffersink_get_frame(m_bufferSinkCtx, dstFrame))
	{
		av_frame_free(&dstFrame);
		return nullptr;
	}

	return dstFrame;
}
