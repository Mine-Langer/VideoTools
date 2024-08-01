#include "VideoFilter.h"
#include <QFile>

typedef struct {
	uint8_t* buffer;
	size_t buffer_size;
	size_t current_pos;
} BufferData;

CVideoFilter::CVideoFilter()
{
	
}

CVideoFilter::~CVideoFilter()
{
	Release();
}

bool CVideoFilter::Init(int nWidth, int nHeight, AVPixelFormat pix_fmt, AVRational sampleRatio, AVRational timebase)
{
	char szArgs[512] = { 0 };
	
	m_VideoWidth = nWidth;
	m_VideoHeight = nHeight;
	m_WaterWidth = m_pWaterMaskFrame->width;
	m_WaterHeight = m_pWaterMaskFrame->height;

	m_filterGraph = avfilter_graph_alloc();

	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	const AVFilter* overlay = avfilter_get_by_name("overlay");
//	const AVFilter* filterDrawText = avfilter_get_by_name("drawtext");

	m_bufferSrcCtx = avfilter_graph_alloc_filter(m_filterGraph, buffersrc, "in");
	m_watermarkSrcCtx = avfilter_graph_alloc_filter(m_filterGraph, buffersrc, "watermark");
	m_bufferSinkCtx = avfilter_graph_alloc_filter(m_filterGraph, buffersink, "out");

	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();

	sprintf_s(szArgs, sizeof(szArgs), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		nWidth, nHeight, pix_fmt, timebase.num, timebase.den, sampleRatio.num, sampleRatio.den);
	avfilter_init_str(m_bufferSrcCtx, szArgs);


	snprintf(szArgs, sizeof(szArgs),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		m_pWaterMaskFrame->width, m_pWaterMaskFrame->height, m_pWaterMaskFrame->format,
		timebase.num, timebase.den, sampleRatio.num, sampleRatio.den);
	avfilter_init_str(m_watermarkSrcCtx, szArgs);

	avfilter_init_str(m_bufferSinkCtx, NULL);

	// 动态设置滤镜位置
	int x=10, y=10; // 10,10 固定左上位置
	if (m_bRandomPosition)
		get_random_position(m_VideoWidth, m_VideoHeight, m_WaterWidth, m_WaterHeight, x, y);
	sprintf_s(szArgs, sizeof(szArgs), "%d:%d", x, y);

	AVFilterContext* overlay_ctx = avfilter_graph_alloc_filter(m_filterGraph, overlay, "overlay");
	avfilter_init_str(overlay_ctx, szArgs);

	avfilter_link(m_bufferSrcCtx, 0, overlay_ctx, 0);
	avfilter_link(m_watermarkSrcCtx, 0, overlay_ctx, 1);
	avfilter_link(overlay_ctx, 0, m_bufferSinkCtx, 0);

	if (avfilter_graph_config(m_filterGraph, NULL) < 0) {
		fprintf(stderr, "Could not configure filter graph\n");
		return false;
	}

	return true;
}

void CVideoFilter::SetFilter(const char* szFilter)
{
	m_szFilter = szFilter;
}

AVFrame* CVideoFilter::Convert(AVFrame* srcFrame, bool& bRet)
{
	if (m_bRandomPosition)	
	{	// 随机变更水印位置
		char overlay_args[8] = { 0 };
		int x, y;
		get_random_position(m_VideoWidth, m_VideoHeight, m_WaterWidth, m_WaterHeight, x, y);
		snprintf(overlay_args, sizeof(overlay_args), "%d:%d", x, y);
		avfilter_graph_send_command(m_filterGraph, "overlay", "x", overlay_args, NULL, 0, 0);
	}

	if (av_buffersrc_add_frame(m_bufferSrcCtx, srcFrame) < 0) {
		return nullptr;
	}

	if (0 > av_buffersrc_add_frame_flags(m_watermarkSrcCtx, m_pWaterMaskFrame, AV_BUFFERSRC_FLAG_KEEP_REF))
	{
		return nullptr;
	}

	while (1)
	{
		AVFrame* dstFrame = av_frame_alloc();
		int ret = av_buffersink_get_frame(m_bufferSinkCtx, dstFrame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_frame_free(&dstFrame);
			break;
		}
		if (ret < 0) {
			av_frame_free(&dstFrame);
			break;
		}
		
		bRet = true;
		return dstFrame;
	}
	return srcFrame;
}

bool CVideoFilter::InitWaterMask()
{	
	AVCodecContext* codec_ctx = NULL;
	const AVCodec* codec = NULL;
	AVPacket packet;

	// 打开 Qt 资源文件
	QFile file(":/HLCapture/res/watermask.png");
	if (!file.open(QIODevice::ReadOnly)) {
		return nullptr;
	}

	QByteArray fileData = file.readAll();
	file.close();

	AVFormatContext* fmt_ctx = open_input_from_memory((uint8_t*)fileData.data(), fileData.size());
	if (!fmt_ctx) {
		return false;
	}

	int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_stream_index < 0)
		return false;

	codec = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "Could not find codec\n");
		return NULL;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		fprintf(stderr, "Could not allocate codec context\n");
		return NULL;
	}

	if (avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar) < 0) {
		fprintf(stderr, "Could not copy codec parameters to context\n");
		return NULL;
	}

	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return NULL;
	}

	m_pWaterMaskFrame = av_frame_alloc();
	if (!m_pWaterMaskFrame) {
		fprintf(stderr, "Could not allocate frame\n");
		return NULL;
	}

	while (av_read_frame(fmt_ctx, &packet) >= 0) {
		if (packet.stream_index == video_stream_index) {
			if (avcodec_send_packet(codec_ctx, &packet) == 0) {
				if (avcodec_receive_frame(codec_ctx, m_pWaterMaskFrame) == 0) 
				{
					break;
				}
			}
		}
		av_packet_unref(&packet);
	}

	av_packet_unref(&packet);
	avcodec_free_context(&codec_ctx);
	avformat_close_input(&fmt_ctx);

	return true;
}

void CVideoFilter::Release()
{
	if (m_pWaterMaskFrame)
	{
		av_freep(&m_pWaterMaskFrame->data[0]);
		av_frame_free(&m_pWaterMaskFrame);
	}

	if (m_filterGraph)
	{
		avfilter_graph_free(&m_filterGraph);
		m_filterGraph = nullptr;
	}
}

AVFormatContext* CVideoFilter::open_input_from_memory(uint8_t* buffer, size_t buffer_size)
{
	AVFormatContext* fmt_ctx = NULL;
	AVIOContext* avio_ctx = NULL;
	uint8_t* avio_ctx_buffer = NULL;
	size_t avio_ctx_buffer_size = 4096;
	BufferData bd = { buffer, buffer_size, 0 };

	fmt_ctx = avformat_alloc_context();
	if (!fmt_ctx) {
		return nullptr;
	}

	avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
	if (!avio_ctx_buffer) {
		avformat_free_context(fmt_ctx);
		return nullptr;
	}

	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &bd, read_packet, NULL, seek_packet);
	if (!avio_ctx) {
		av_free(avio_ctx_buffer);
		avformat_free_context(fmt_ctx);
		return nullptr;
	}

	fmt_ctx->pb = avio_ctx;
	fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

	int ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
	if (ret < 0) {
		avio_context_free(&avio_ctx);
		avformat_free_context(fmt_ctx);
		return nullptr;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		avformat_close_input(&fmt_ctx);
		avio_context_free(&avio_ctx);
		return nullptr;
	}

	return fmt_ctx;
}

int CVideoFilter::read_packet(void* opaque, uint8_t* buf, int buf_size)
{
	BufferData* bd = (BufferData*)opaque;
	buf_size = FFMIN(buf_size, bd->buffer_size - bd->current_pos);
	if (!buf_size)
		return AVERROR_EOF;
	memcpy(buf, bd->buffer + bd->current_pos, buf_size);
	bd->current_pos += buf_size;
	return buf_size;
}

int64_t CVideoFilter::seek_packet(void* opaque, int64_t offset, int whence)
{
	BufferData* bd = (BufferData*)opaque;
	if (whence == AVSEEK_SIZE)
		return bd->buffer_size;
	if (whence == SEEK_SET)
		bd->current_pos = offset;
	else if (whence == SEEK_CUR)
		bd->current_pos += offset;
	else if (whence == SEEK_END)
		bd->current_pos = bd->buffer_size + offset;
	return bd->current_pos;
}

void CVideoFilter::get_random_position(int video_width, int video_height, int watermark_width, int watermark_height, int& x, int& y)
{
	x = rand() % (video_width - watermark_width);
	y = rand() % (video_height - watermark_height);
}
