#include "ImageConvert.h"

ImageConvert::ImageConvert()
{
}

ImageConvert::~ImageConvert()
{
}

bool ImageConvert::Open(const char* szFile)
{
	if (0 != avformat_open_input(&InputFormatCtx, szFile, nullptr, nullptr))	
		return false;
	
	InputFormatCtx->iformat->name;

	if (0 > avformat_find_stream_info(InputFormatCtx, nullptr))
		return false;
	
	int imgIndex = av_find_best_stream(InputFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (imgIndex < 0)
		return false;

	AVStream* pStream = InputFormatCtx->streams[imgIndex];
	InputCodecCtx = avcodec_alloc_context3(nullptr);
	if (0 > avcodec_parameters_to_context(InputCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(InputCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(InputCodecCtx, pCodec, nullptr))
		return false;

	m_srcWidth = InputCodecCtx->width;
	m_srcHeight = InputCodecCtx->height;

	return true;
}

bool ImageConvert::SetOutput()
{
	if (0 > avformat_alloc_output_context2(&OutputFormatCtx, nullptr, nullptr, "output.ico"))
		return false;

	const AVOutputFormat* outputFormat = OutputFormatCtx->oformat;
	const AVCodec* pCodec = avcodec_find_encoder(outputFormat->video_codec);

	OutputCodecCtx = avcodec_alloc_context3(pCodec);
	OutputCodecCtx->width = m_srcWidth;
	OutputCodecCtx->height = m_srcHeight;


	return true;
}

void ImageConvert::Work()
{
	AVPacket* pkt = av_packet_alloc();

	av_read_frame(InputFormatCtx, );
}
