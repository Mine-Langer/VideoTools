#include "VideoDecoder.h"

CVideoDecoder::CVideoDecoder()
{
}

CVideoDecoder::~CVideoDecoder()
{

}

bool CVideoDecoder::Open(const std::string& szFile)
{
	return true;
}

bool CVideoDecoder::Open(CDemultiplexer& demux)
{
	AVStream* pStream = demux.GetFormatCtx()->streams[demux.GetVideoIdx()];

	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec) return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_timebase = av_q2d(pStream->time_base);
	m_duration = m_timebase * (pStream->duration * 1.0);
	m_rate = av_q2d(pStream->avg_frame_rate);

	return true;
}

void CVideoDecoder::Start(IDecoderEvent* pEvt)
{
	m_DecoderEvt = pEvt;

	m_bRun = true;
	m_thread = std::thread(&CVideoDecoder::Work, this);
}

void CVideoDecoder::Close()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();

	while (true)
	{
		AVPacket* pkt = nullptr;
		if (m_pktQueue.Pop(pkt))
		{
			if (pkt)
				av_packet_free(&pkt);
		}
		else
		{
			break;
		}
	}

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

bool CVideoDecoder::SendPacket(AVPacket* pkt)
{
	AVPacket* tpkt = nullptr;
	if (pkt)
		tpkt = av_packet_clone(pkt);

	m_pktQueue.MaxSizePush(tpkt, &m_bRun);

	return true;
}

bool CVideoDecoder::InitSwsContext(int w, int h, enum AVPixelFormat pix_fmt)
{
	m_swsWidth = w == -1 ? m_pCodecCtx->width : w;
	m_swsHeight = h == -1 ? m_pCodecCtx->height : h;
	m_swsPixFmt = pix_fmt == AV_PIX_FMT_NONE ? m_pCodecCtx->pix_fmt : pix_fmt;

	m_pSwsCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
		m_swsWidth, m_swsHeight, m_swsPixFmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
	if (!m_pSwsCtx)
		return false;

	return true;
}

double CVideoDecoder::GetTimebase() const
{
	return m_timebase;
}

void CVideoDecoder::Work()
{
	int ret = 0;
	AVPacket* pkt = nullptr;
	AVFrame* srcFrame = av_frame_alloc();

	while (m_bRun)
	{
		if (m_pktQueue.Pop(pkt))
		{
			if (pkt == nullptr)
			{
				if (m_DecoderEvt) m_DecoderEvt->VideoEvent(nullptr, nullptr);
				m_bRun = false;
			}
			else
			{
				if (0 > avcodec_send_packet(m_pCodecCtx, pkt))
				{
					m_bRun = false;
					av_packet_free(&pkt);
					break;
				}
				else
				{
					// 解码器解码得到原始YUV帧
					while (ret = avcodec_receive_frame(m_pCodecCtx, srcFrame), m_bRun)
					{
						if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
							break;
						else if (ret < 0)
							m_bRun = false;
						else if (ret == 0)
						{
							AVFrame* dstFrame = av_frame_alloc();
							dstFrame->format = m_swsPixFmt;
							dstFrame->width = m_swsWidth;
							dstFrame->height = m_swsHeight;
							if (0 > av_frame_get_buffer(dstFrame, 0))
							{
								av_frame_free(&dstFrame);
								m_bRun = false;
								break;
							}
							sws_scale(m_pSwsCtx, srcFrame->data, srcFrame->linesize, 0, m_pCodecCtx->height, dstFrame->data, dstFrame->linesize);

							dstFrame->pts = srcFrame->pts;
							dstFrame->best_effort_timestamp = srcFrame->pts;

							if (m_DecoderEvt)
								m_DecoderEvt->VideoEvent(dstFrame, nullptr);

							av_frame_free(&dstFrame);
							av_frame_unref(srcFrame);
						}
					}
				}
				av_packet_free(&pkt);		
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}	
	}

	av_frame_free(&srcFrame);
	printf("Video Image Data Decode overed.......\n");
}
