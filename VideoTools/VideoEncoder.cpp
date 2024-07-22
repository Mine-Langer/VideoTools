#include "VideoEncoder.h"

CVideoEncoder::CVideoEncoder()
{

}

CVideoEncoder::~CVideoEncoder()
{

}


AVCodecContext* CVideoEncoder::Init(enum AVCodecID codec_id, int width, int height)
{
	const AVCodec* pCodec = avcodec_find_encoder(codec_id);
	if (!pCodec)
		return nullptr;

	m_pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!m_pCodecCtx)
		return nullptr;

	m_pCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
	m_pCodecCtx->width = width;
	m_pCodecCtx->height = height;
	m_pCodecCtx->codec_id = codec_id;
	m_pCodecCtx->bit_rate = 4000000;
	m_pCodecCtx->time_base = { 1, 25 };
	m_pCodecCtx->gop_size = 12;
	m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	av_opt_set(m_pCodecCtx->priv_data, "b-pyramid", "none", 0);
	av_opt_set(m_pCodecCtx->priv_data, "preset", "superfast", 0);
	av_opt_set(m_pCodecCtx->priv_data, "tune", "zerolatency", 0);

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return nullptr;

	return m_pCodecCtx;
}

void CVideoEncoder::Start(IRemuxEvent* pEvt)
{
	m_pRemuxEvt = pEvt;
	
	m_bRun = true;
	m_thread = std::thread(&CVideoEncoder::Work, this);
}

void CVideoEncoder::Close()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();
}

void CVideoEncoder::SendFrame(AVFrame* srcFrame)
{
	AVFrame* tframe = nullptr;
	if (srcFrame)
		tframe = av_frame_clone(srcFrame);
	m_videoDataQueue.MaxSizePush(tframe, &m_bRun);
}

void CVideoEncoder::Work()
{
	int ret = 0;
	AVFrame* pFrame = nullptr;
	AVPacket* pkt = av_packet_alloc();

	while (m_bRun)
	{
		if (m_videoDataQueue.Pop(pFrame))
		{
			if (pFrame == nullptr)
			{
				m_pRemuxEvt->RemuxEvent(nullptr, AVMEDIA_TYPE_VIDEO, m_pts);
				m_bRun = false;
			}
			else
			{				
				pFrame->pts = m_pts++;
				if (0 == avcodec_send_frame(m_pCodecCtx, pFrame))
				{
					
					if (0 == avcodec_receive_packet(m_pCodecCtx, pkt))
					{
						//av_packet_rescale_ts(pkt, m_pCodecCtx->time_base, m_pStream->time_base);
						//pkt->stream_index = m_pStream->index;
						m_pRemuxEvt->RemuxEvent(pkt, AVMEDIA_TYPE_VIDEO, m_pts);
					}
				}
				av_packet_unref(pkt);
				av_frame_free(&pFrame);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}		
	}
	av_packet_free(&pkt);
	printf("Video Image Data Encode overed .......\n");
}
