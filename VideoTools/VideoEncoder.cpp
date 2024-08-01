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
	m_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	m_pCodecCtx->bit_rate = 4000000;
	m_pCodecCtx->time_base = { 1, 25 };	// 帧率 25 fps
	m_pCodecCtx->framerate = { 25, 1 }; // 帧率 25 fps
	//m_pCodecCtx->gop_size = 10;			// 每10帧插入一个I帧
	m_pCodecCtx->max_b_frames = 1;		// 最大B帧数
	m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (m_pCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		av_opt_set(m_pCodecCtx->priv_data, "profile", "main", 0);
		av_opt_set(m_pCodecCtx->priv_data, "b-pyramid", "none", 0);
		av_opt_set(m_pCodecCtx->priv_data, "preset", "superfast", 0);
		av_opt_set(m_pCodecCtx->priv_data, "tune", "zerolatency", 0);
	}

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

	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
	}
}

void CVideoEncoder::SendFrame(AVFrame* srcFrame)
{
	AVFrame* tframe = nullptr;
	if (srcFrame)
		tframe = av_frame_clone(srcFrame);
	m_videoDataQueue.MaxSizePush(tframe, &m_bRun);
}

void CVideoEncoder::SetStreams(AVStream* pStream)
{
	m_pStream = pStream;
}

void CVideoEncoder::Work()
{
	int ret = 0;
	AVFrame* pFrame = nullptr;
	AVPacket* pkt = av_packet_alloc();
	m_pts = 0; // 为每个帧设置一个唯一的时间戳

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
				pFrame->pkt_dts = pFrame->pts = av_rescale_q_rnd(m_pts, m_pCodecCtx->time_base, m_pStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX))*2;
				pFrame->duration = av_rescale_q_rnd(1, m_pCodecCtx->time_base, m_pStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				
				avcodec_send_frame(m_pCodecCtx, pFrame);

				while (true)
				{
					ret = avcodec_receive_packet(m_pCodecCtx, pkt);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
						break;
					else if (ret < 0)
						break;
					else
					{
						m_pts++;
						//av_packet_rescale_ts(pkt, m_pCodecCtx->time_base, m_pStream->time_base);
						pkt->stream_index = m_pStream->index;
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
