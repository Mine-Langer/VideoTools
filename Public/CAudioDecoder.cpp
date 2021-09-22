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

	SrcFrame = av_frame_alloc();

	return true;
}

void CAudioDecoder::Start()
{
	m_bRun = true;
	m_decodeThread = std::thread(&CAudioDecoder::OnDecodeFunction, this);
}

void CAudioDecoder::SendPacket(AVPacket* pkt)
{
	AVPacket* apkt = av_packet_clone(pkt);
	AudioPacketQueue.MaxSizePush(apkt);
}

void CAudioDecoder::OnDecodeFunction()
{
	int error = 0;
	AVPacket* apkt = nullptr;

	while (m_bRun)
	{
		if (!AudioPacketQueue.Pop(apkt))
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			if (0 > avcodec_send_packet(AudioCodecCtx, apkt))
				break;

			error = avcodec_receive_frame(AudioCodecCtx, SrcFrame);
			if (error < 0) {
				if (error == AVERROR(EAGAIN) || error == AVERROR_EOF)
					continue;
				break;
			}
			else if (error == 0)
			{

			}
			av_frame_unref(SrcFrame);
		}
		av_packet_free(&apkt);
	}
}
