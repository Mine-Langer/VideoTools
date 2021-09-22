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

	SrcFrame = av_frame_alloc();

	return true;
}

void CVideoDecoder::Start()
{
	m_bRun = true;
	m_decodeThread = std::thread(&CVideoDecoder::OnDecodeFunction, this);
}

void CVideoDecoder::SendPacket(AVPacket* pkt)
{
	AVPacket* vpkt = av_packet_clone(pkt);
	VideoPacketQueue.MaxSizePush(vpkt);
}

void CVideoDecoder::OnDecodeFunction()
{
	int error = 0;
	AVPacket* vpkt = nullptr;
	while (m_bRun)
	{
		if (!VideoPacketQueue.Pop(vpkt))
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			if (0 > avcodec_send_packet(VideoCodecCtx, vpkt))
				break;

			error = avcodec_receive_frame(VideoCodecCtx, SrcFrame);
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
		av_packet_free(&vpkt);
	}
}
