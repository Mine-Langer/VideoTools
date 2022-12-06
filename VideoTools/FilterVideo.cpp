#include "FilterVideo.h"

CFilterVideo::CFilterVideo()
{

}

CFilterVideo::~CFilterVideo()
{

}

bool CFilterVideo::Init(int nWidth, int nHeight, AVPixelFormat pix_fmt, AVRational sampleRatio, AVRational timebase)
{
	char szArgs[512] = { 0 };
	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	//const AVFilter* filterDrawText = avfilter_get_by_name("drawtext");

	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();

	AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

	sprintf_s(szArgs, sizeof(szArgs), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		nWidth/2*2, nHeight/2*2, pix_fmt, timebase.num, timebase.den, sampleRatio.num, sampleRatio.den);

	m_filterGraph = avfilter_graph_alloc();
	if (0 > avfilter_graph_create_filter(&m_bufferSrcCtx, buffersrc, "in", szArgs, nullptr, m_filterGraph))
		return false;

	//AVFilterContext* drawTextFilterCtx = nullptr;
	//if (0 > avfilter_graph_create_filter(&drawTextFilterCtx, filterDrawText, "drawtext", m_szFilter.c_str(), nullptr, m_filterGraph))
	//	return false;

	if (0 > avfilter_graph_create_filter(&m_bufferSinkCtx, buffersink, "out", nullptr, nullptr, m_filterGraph))
		return false;
	
	av_opt_set_int_list(m_bufferSinkCtx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

	//if (0 > avfilter_link(m_bufferSrcCtx, 0, drawTextFilterCtx, 0))
	//	return false;
	//
	//if (0 > avfilter_link(drawTextFilterCtx, 0, m_bufferSrcCtx, 0))
	//	return false;

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

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

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
	if (0 > av_buffersrc_add_frame_flags(m_bufferSrcCtx, srcFrame, AV_BUFFERSRC_FLAG_KEEP_REF))
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
