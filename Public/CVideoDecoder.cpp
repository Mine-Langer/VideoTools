#include "CVideoDecoder.h"

CVideoDecoder::CVideoDecoder()
{

}

CVideoDecoder::~CVideoDecoder()
{

}

bool CVideoDecoder::Open(AVStream* pStream)
{
	VideoCodecCtx = avcodec_alloc_context3(nullptr);
	if (VideoCodecCtx == nullptr)
		return false;

	if (0 > avcodec_parameters_to_context(VideoCodecCtx, pStream->codecpar))
		return false;

	AVCodec* codec = avcodec_find_decoder(VideoCodecCtx->codec_id);
	if (codec == nullptr)
		return false;

	if (0 > avcodec_open2(VideoCodecCtx, codec, nullptr))
		return false;

	m_timebase = av_q2d(pStream->time_base);
	m_duration = m_timebase * (pStream->duration * 1.0);
	m_rate = av_q2d(pStream->avg_frame_rate);

	SrcFrame = av_frame_alloc();

	return true;
}

void CVideoDecoder::Start(IDecoderEvent* evt)
{
	m_event = evt;
	m_bRun = true;
	m_decodeThread = std::thread(&CVideoDecoder::OnDecodeFunction, this);
}

bool CVideoDecoder::SetConfig(int width, int height, AVPixelFormat iformat, int iflags)
{
	VideoWidth = width;
	VideoHeight = height;
	VideoFormat = iformat;
	SwsCtx = sws_getContext(VideoCodecCtx->width, VideoCodecCtx->height, VideoCodecCtx->pix_fmt, width, height, iformat, iflags, nullptr, nullptr, nullptr);
	if (SwsCtx == nullptr)
		return false;

	return true;
}

bool CVideoDecoder::SendPacket(AVPacket* pkt)
{
	AVPacket* vpkt = av_packet_clone(pkt);
	m_packetQueue.MaxSizePush(vpkt);

	return true;
}

void CVideoDecoder::OnDecodeFunction()
{
	int error = 0;
	AVPacket* vpkt = nullptr;
	while (m_bRun)
	{
		if (!m_packetQueue.Pop(vpkt))
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
				DstFrame = av_frame_alloc();
				DstFrame->width = VideoWidth;
				DstFrame->height = VideoHeight;
				DstFrame->format = VideoFormat;
				av_frame_get_buffer(DstFrame, 0);
				sws_scale(SwsCtx, SrcFrame->data, SrcFrame->linesize, 0, VideoCodecCtx->height, DstFrame->data, DstFrame->linesize);
				DstFrame->pts = SrcFrame->pts;
				DstFrame->best_effort_timestamp = SrcFrame->best_effort_timestamp;
				m_event->VideoEvent(DstFrame);
				av_frame_free(&DstFrame);
			}
			av_frame_unref(SrcFrame);
		}
		av_packet_free(&vpkt);
	}
}

void CVideoDecoder::Close()
{

}
