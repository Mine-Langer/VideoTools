#include "CVideoEncoder.h"

CVideoEncoder::CVideoEncoder()
{

}

CVideoEncoder::~CVideoEncoder()
{

}

bool CVideoEncoder::InitConfig(AVFormatContext* outputFmtCtx)
{
	AVStream* pStream = avformat_new_stream(outputFmtCtx, nullptr);
	if (pStream == nullptr)
		return false;

	AVCodec* codec = avcodec_find_encoder(outputFmtCtx->oformat->video_codec);
	if (codec == nullptr)
		return true;

	VideoCodecCtx = avcodec_alloc_context3(codec);
	if (VideoCodecCtx == nullptr)
		return false;



	return true;
}
