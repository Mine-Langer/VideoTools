#include "VideoDecoder.h"

CVideoDecoder::CVideoDecoder()
{

}

CVideoDecoder::~CVideoDecoder()
{

}

bool CVideoDecoder::Open(AVStream* pStream)
{
	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (!m_pCodecCtx)
		return false;

	AVCodec* pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_timebase = av_q2d(pStream->time_base);
	m_duration = m_timebase * pStream->duration;

	return true;
}

void CVideoDecoder::Start(IDecoderEvent* pEvent)
{
	m_pEvent = pEvent;
	m_avStatus = ePlay;
	m_decodeThread = std::thread(&CVideoDecoder::OnDecodeThread, this);
}

void CVideoDecoder::Close()
{
	m_avStatus = eStop;
	if (m_decodeThread.joinable())
		m_decodeThread.join();

	if (m_pSwsCtx)
	{
		sws_freeContext(m_pSwsCtx);
		m_pSwsCtx = nullptr;
	}

	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}
}

void CVideoDecoder::SetConfig(SDL_Rect& rect, int dstWidth, int dstHeight)
{
	float ratio = (m_pCodecCtx->width*1.0) / (m_pCodecCtx->height*1.0);
	m_swsWidth = dstWidth / 2 * 2;
	m_swsHeight = m_swsWidth * 1.0 / ratio;
	if (m_swsHeight > dstHeight)
		m_swsWidth = dstHeight * ratio;

	rect.x = (dstWidth - m_swsWidth) / 2;
	rect.y = (dstHeight - m_swsHeight) / 2;
	rect.w = m_swsWidth;
	rect.h = m_swsHeight;

	m_swsWidth = dstWidth;
	m_swsHeight = dstHeight;
	m_swsFormat = AV_PIX_FMT_YUV420P;

	if (m_pSwsCtx) {
		sws_freeContext(m_pSwsCtx);
		m_pSwsCtx = nullptr;
	}
	m_pSwsCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
		m_swsWidth, m_swsHeight, m_swsFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);
}

AVFrame* CVideoDecoder::ConvertFrame(AVFrame* frame) 
{
	AVFrame* swsFrame = av_frame_alloc();
	swsFrame->width = m_swsWidth;
	swsFrame->height = m_swsHeight;
	swsFrame->format = m_swsFormat;
	int ret = av_frame_get_buffer(swsFrame, 0);
	ret = sws_scale(m_pSwsCtx, frame->data, frame->linesize, 0, frame->height, swsFrame->data, swsFrame->linesize);

	swsFrame->pts = frame->pts;
	swsFrame->best_effort_timestamp = frame->best_effort_timestamp;
	swsFrame->pkt_dts = frame->pts * m_timebase;

	av_frame_free(&frame);

	return swsFrame;
}

void CVideoDecoder::PushPacket(AVPacket* pkt)
{
	AVPacket* vPkt = nullptr;
	if (pkt)
		vPkt = av_packet_clone(pkt);

	bool bRun = (m_avStatus == eStop);
	m_vPktQueue.MaxSizePush(vPkt, &bRun);
}

void CVideoDecoder::OnDecodeThread()
{
	AVPacket* packet = nullptr;
	int err = 0;

	while (m_avStatus != eStop)
	{
		if (!m_vPktQueue.Pop(packet))
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		else
		{
			if (packet == nullptr)
			{
				m_pEvent->VideoEvent(nullptr);
				break;
			}
			else
			{
				err = avcodec_send_packet(m_pCodecCtx, packet);
				if (err < 0)
				{
					av_packet_free(&packet);
					continue;
				}

				AVFrame* frame = av_frame_alloc();
				err = avcodec_receive_frame(m_pCodecCtx, frame);
				if (err < 0)
				{
					av_packet_free(&packet);
					continue;
				}

				m_pEvent->VideoEvent(frame);

				av_packet_free(&packet);				
			}
		}
	}
}
