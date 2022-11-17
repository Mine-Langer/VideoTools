#include "VideoEncoder.h"

CVideoEncoder::CVideoEncoder()
{

}

CVideoEncoder::~CVideoEncoder()
{

}

bool CVideoEncoder::Init(AVFormatContext* outFmtCtx, enum AVCodecID codec_id, int width, int height)
{
	const AVCodec* pCodec = avcodec_find_encoder(codec_id);
	if (!pCodec)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!m_pCodecCtx)
		return false;

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

	if (outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
		m_pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_pStream = avformat_new_stream(outFmtCtx, nullptr);
	if (!m_pStream)
		return false;

	m_pStream->time_base = m_pCodecCtx->time_base;
	m_pStream->id = outFmtCtx->nb_streams - 1;

	if (0 > avcodec_parameters_from_context(m_pStream->codecpar, m_pCodecCtx))
		return false;

	return true;
}

AVPacket* CVideoEncoder::Encode(AVFrame* srcFrame)
{
	int error = 0;

	error = avcodec_send_frame(m_pCodecCtx, srcFrame);
	if (error < 0)
		return nullptr;

	AVPacket* packet = av_packet_alloc();
	error = avcodec_receive_packet(m_pCodecCtx, packet);
	if (error < 0)
	{
		av_packet_free(&packet);
		return nullptr;
	}

	av_packet_rescale_ts(packet, m_pCodecCtx->time_base, m_pStream->time_base);
	packet->stream_index = m_pStream->index;

	return packet;
}


void CVideoEncoder::Release()
{
	if (m_pCodecCtx) {
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}
}

AVRational CVideoEncoder::GetTimeBase()
{
	return m_pCodecCtx->time_base;
}
