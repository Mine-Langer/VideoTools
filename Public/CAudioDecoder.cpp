#include "CAudioDecoder.h"

CAudioDecoder::CAudioDecoder()
{

}

CAudioDecoder::~CAudioDecoder()
{

}

bool CAudioDecoder::Open(AVStream* pStream, enum AVCodecID codecId)
{
	AVCodec* codec = avcodec_find_decoder(codecId);
	if (codec == nullptr)
		return false;

	AudioCodecCtx = avcodec_alloc_context3(codec);
	if (AudioCodecCtx == nullptr)
		return false;

	if (0 > avcodec_parameters_to_context(AudioCodecCtx, pStream->codecpar))
		return false;

	if (0 > avcodec_open2(AudioCodecCtx, codec, nullptr))
		return false;

	return true;
}
