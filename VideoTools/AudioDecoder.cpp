#include "AudioDecoder.h"

CAudioDecoder::CAudioDecoder()
{

}

CAudioDecoder::~CAudioDecoder()
{

}

bool CAudioDecoder::Open(const std::string& strFile)
{
	return true;
}

bool CAudioDecoder::Open(CDemultiplexer& demux)
{
	AVStream* pStream = demux.GetFormatCtx()->streams[demux.GetAudioIdx()];

	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (!m_pCodecCtx)
		return false;

	m_pCodecCtx->pkt_timebase = pStream->time_base;
	avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar);

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_timebase = av_q2d(pStream->time_base);
	m_duration = m_timebase * (pStream->duration * 1.0);
	m_rate = m_pCodecCtx->sample_rate;
	
	return true;
}

void CAudioDecoder::Start(IDecoderEvent* pEvt)
{
	m_DecoderEvt = pEvt;

	m_bRun = true;
	m_DecodeThread = std::thread(&CAudioDecoder::Work, this);
}

void CAudioDecoder::Close()
{
	m_bRun = false;
	if (m_DecodeThread.joinable())
		m_DecodeThread.join();

	while (true)
	{
		AVPacket* pkt = nullptr;
		if (m_PktQueue.Pop(pkt))
		{
			if (pkt)
				av_packet_free(&pkt);
		}
		else
		{
			break;
		}
	}

	if (m_pSwrCtx)
	{
		swr_close(m_pSwrCtx);
		swr_free(&m_pSwrCtx);
		m_pSwrCtx = nullptr;
	}

	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}
}

void CAudioDecoder::GetSrcParameter(int& sample_rate, AVChannelLayout& ch_layout, enum AVSampleFormat& sample_fmt)
{
	sample_rate = m_pCodecCtx->sample_rate;
	ch_layout = m_pCodecCtx->ch_layout;
	sample_fmt = m_pCodecCtx->sample_fmt;
}

bool CAudioDecoder::SetSwr(AVChannelLayout ch_layout, enum AVSampleFormat sample_fmt, int sample_rate, int& nb_samples)
{
	m_swr_ch_layout = ch_layout;
	m_swr_sample_fmt = sample_fmt;
	m_swr_sample_rate = sample_rate;

	if (0 > swr_alloc_set_opts2(&m_pSwrCtx, &m_swr_ch_layout, m_swr_sample_fmt, m_swr_sample_rate,
		&m_pCodecCtx->ch_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr))
		return false;
	
	if (!m_pSwrCtx || 0 > swr_init(m_pSwrCtx))
		return false;

	// 这是ffmepg官方示例提供的计算重采样之后的 单帧单声道采样数 的方法
	// 注意，frame_size 是原始的 单帧单声道采样数 不是原始帧的byte个数
	// 例如AV_SAMPLE_FMT_S16，单帧如有1024个采样数，则单帧byte个数是1024*声道数*sizeof(short)
	// 单帧单声道采样数和采样率相关
	m_swr_nb_samples = (int)av_rescale_rnd(
		swr_get_delay(m_pSwrCtx, m_pCodecCtx->sample_rate) + m_pCodecCtx->frame_size,
		m_swr_sample_rate, m_pCodecCtx->sample_rate, AV_ROUND_INF);
	
	nb_samples = m_swr_nb_samples;

	return true;
}

void CAudioDecoder::Work()
{
	int ret = 0;
	AVPacket* pkt = nullptr;
	AVFrame* srcFrame = av_frame_alloc();

	while (m_bRun)
	{
		if (m_PktQueue.Pop(pkt))
		{
			if (pkt == nullptr)
			{
				if (m_DecoderEvt)
					m_DecoderEvt->AudioEvent(nullptr);
				m_bRun = false; // 结束
			}
			else
			{
				if (0 > avcodec_send_packet(m_pCodecCtx, pkt))
					m_bRun = false;
				else
				{
					// 解码器解码得到原始PCM帧
					while (ret = avcodec_receive_frame(m_pCodecCtx, srcFrame), m_bRun)
					{
						if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
							break;
						else if (ret < 0)
							m_bRun = false;
						else if (ret == 0)
						{
							/*uint8_t** dst_buf = NULL;
							int dst_buf_size = 0;
							
							// 为重采样目标PCM帧分配存储空间
							dst_buf = (uint8_t**)calloc(m_swr_ch_layout.nb_channels, sizeof(uint8_t*));

							if (0 > av_samples_alloc(dst_buf, &dst_buf_size,
								m_swr_ch_layout.nb_channels, srcFrame->nb_samples, m_swr_sample_fmt, 1))
							{
								m_bRun = false;
							}
							else
							{
								// 重采样并填充buffer
								ret = swr_convert(m_pSwrCtx, dst_buf, srcFrame->nb_samples, (const uint8_t**)srcFrame->data, srcFrame->nb_samples);

								if (m_DecoderEvt)
									m_DecoderEvt->AudioEvent(dst_buf, ret, srcFrame->best_effort_timestamp, m_timebase, m_rate);
							}
							av_freep(&dst_buf[0]);
							free(dst_buf);*/

							//为重采样目标PCM帧分配存储空间
							AVFrame* dstFrame = av_frame_alloc();
							dstFrame->nb_samples = srcFrame->nb_samples;
							dstFrame->format = m_swr_sample_fmt;
							dstFrame->sample_rate = m_swr_sample_rate;
							av_channel_layout_copy(&dstFrame->ch_layout, &m_swr_ch_layout);

							if (0 > av_frame_get_buffer(dstFrame, 0))
							{
								av_frame_free(&dstFrame);
								m_bRun = false;
								break;
							}

							if (0 > swr_convert(m_pSwrCtx, dstFrame->data, dstFrame->nb_samples, (const uint8_t**)srcFrame->data, srcFrame->nb_samples))
							{
								av_frame_free(&dstFrame);
								m_bRun = false;
								break;
							}
							dstFrame->pts = srcFrame->pts;
							dstFrame->best_effort_timestamp = srcFrame->best_effort_timestamp;

							// int realtime = pkt->pts * av_q2d(m_pCodecCtx->time_base);
							// printf("audio decoder -> pts:%lld  realtime:%d  .\r\n", pkt->pts, realtime);

							if (m_DecoderEvt)
								m_DecoderEvt->AudioEvent(dstFrame);

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
	printf("Input file audio decoding completed.......\r\n");
}

bool CAudioDecoder::SendPacket(AVPacket* pkt)
{
	AVPacket* tpkt = nullptr;
	if (pkt)
		tpkt = av_packet_clone(pkt);
	m_PktQueue.MaxSizePush(tpkt, &m_bRun);

	return true;
}

double CAudioDecoder::GetTimebase() const
{
	return m_timebase;
}
