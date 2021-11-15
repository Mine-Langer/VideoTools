#include "FilterVideo.h"

CFilterVideo::CFilterVideo()
{

}

CFilterVideo::~CFilterVideo()
{

}

bool CFilterVideo::Init()
{
	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();
//	AVRational timebase = format_ctx->streams[video_stream]->time_base;
	AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
}
