#include "CVideoDecoder.h"

CVideoDecoder::CVideoDecoder()
{

}

CVideoDecoder::~CVideoDecoder()
{

}

bool CVideoDecoder::Open(AVStream* pStream, enum AVCodecID codecId)
{
	AVCodec* codec = avcodec_find_decoder(codecId);
	if (codec == nullptr)
		return false;

	VideoCodecCtx = avcodec_alloc_context3(codec);
	if (VideoCodecCtx == nullptr)
		return false;

	if (0 > avcodec_parameters_to_context(VideoCodecCtx, pStream->codecpar))
		return false;
	
	if (0 > avcodec_open2(VideoCodecCtx, codec, nullptr))
		return false;

	return true;
}
