#include "AudioDecoder.h"

CAudioDecoder::CAudioDecoder()
{

}

CAudioDecoder::~CAudioDecoder()
{

}

bool CAudioDecoder::Open(const char* szInput)
{
#if 0
	if (!m_demux.Open(szInput))
		return false;

	AVStream* pStream = m_demux.FormatContext()->streams[m_demux.AudioStreamIndex()];
	if (!pStream)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (!m_pCodecCtx)
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_pCodecCtx->pkt_timebase = pStream->time_base;
#endif

	return true;
}

bool CAudioDecoder::Open(CDemultiplexer* pDemux)
{
	AVStream* pStream = pDemux->FormatContext()->streams[pDemux->AudioStreamIndex()];
	if (!pStream)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (!m_pCodecCtx)
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_pCodecCtx->pkt_timebase = pStream->time_base;
	m_timebase = av_q2d(m_pCodecCtx->time_base);

	return true;
}

bool CAudioDecoder::OpenMicrophone(const char* szUrl)
{
#if 0
	const AVInputFormat* ifmt = av_find_input_format("dshow");
	if (!ifmt)
		return false;

	if (!m_demux.Open(szUrl, ifmt, nullptr))
		return false;

	AVStream* pStream = m_demux.FormatContext()->streams[m_demux.AudioStreamIndex()];
	if (!pStream)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (!m_pCodecCtx)
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;
#endif
	return true;
}

bool CAudioDecoder::Start(IDecoderEvent* pEvt)
{
	m_pEvent = pEvt;
	if (!m_pEvent)
		return false;

	m_bRun = true;
	m_state = Started;
	m_thread = std::thread(&CAudioDecoder::OnDecodeFunction, this);

	return true;
}

void CAudioDecoder::Stop()
{
	m_state = Stopped;
	if (m_thread.joinable())
		m_thread.join();
}

void CAudioDecoder::Clear()
{
	while (!m_srcAPktQueue.Empty())
	{
		AVPacket* pkt = nullptr;
		m_srcAPktQueue.Pop(pkt);
		if (pkt)
			av_packet_free(&pkt);
	}
}

bool CAudioDecoder::WaitFinished()
{
	return true;
}

void CAudioDecoder::SendPacket(AVPacket* pkt)
{
	m_srcAPktQueue.MaxSizePush(pkt, &m_bRun);
}

void CAudioDecoder::Release()
{
	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (m_pSwrCtx)
	{
		swr_free(&m_pSwrCtx);
		m_pSwrCtx = nullptr;
	}

	Clear();
}

void CAudioDecoder::GetSrcParameter(int& sample_rate, AVChannelLayout& ch_layout, enum AVSampleFormat& sample_fmt)
{
	sample_rate = m_pCodecCtx->sample_rate;
	ch_layout = m_pCodecCtx->ch_layout;
	sample_fmt = m_pCodecCtx->sample_fmt;
}

int CAudioDecoder::SetSwrContext(AVChannelLayout ch_layout, enum AVSampleFormat sample_fmt, int sample_rate)
{
	m_swr_ch_layout = ch_layout;
	m_swr_sample_rate = sample_rate;
	m_swr_sample_fmt = sample_fmt;

	if (0 > swr_alloc_set_opts2(&m_pSwrCtx, &m_swr_ch_layout, m_swr_sample_fmt, m_swr_sample_rate,
		&m_pCodecCtx->ch_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr))
		return -1;

	if (0 > swr_init(m_pSwrCtx))
		return -1;

	int nb_samples = (int)av_rescale_rnd(swr_get_delay(m_pSwrCtx, m_pCodecCtx->sample_rate) + m_pCodecCtx->frame_size,
		m_swr_sample_rate, m_pCodecCtx->sample_rate, AV_ROUND_INF);

	return nb_samples;
}


AVChannelLayout CAudioDecoder::GetChannelLayout()
{
	return m_pCodecCtx->ch_layout;
}

double CAudioDecoder::Timebase()
{
	return m_timebase;  
}

void CAudioDecoder::OnDecodeFunction()
{
	int error = 0;
	AVPacket* pkt = nullptr;
	AVFrame* srcFrame = av_frame_alloc();

	while (m_state != Stopped && m_bRun)
	{
		if (m_state == Paused)
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		else
		{
			if (!m_srcAPktQueue.Pop(pkt))
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			else
			{
				if (pkt == nullptr) {
					// ½áÊø±êÖ¾
					printf(" audio decode overed.\n");
					m_pEvent->AudioEvent(nullptr);
					break;
				}
				else
				{
					error = avcodec_send_packet(m_pCodecCtx, pkt);
					if (error < 0)
					{
						av_packet_free(&pkt);
						continue;
					}
					av_packet_free(&pkt);

					error = avcodec_receive_frame(m_pCodecCtx, srcFrame);
					if (error < 0)
					{
						continue;
					}

					AVFrame* cvtFrame = ConvertFrame(srcFrame);

					//printf(" audio decode a frame: pts(%lld)\n", cvtFrame->pts);
					m_pEvent->AudioEvent(cvtFrame);

					av_frame_unref(srcFrame);
				}
			}
		}
	}
	av_frame_free(&srcFrame);

	Release();
}

AVFrame* CAudioDecoder::ConvertFrame(AVFrame* frame)
{
	AVFrame* dstFrame = av_frame_alloc();
	dstFrame->nb_samples = frame->nb_samples;
	dstFrame->format = m_swr_sample_fmt;
	dstFrame->sample_rate = m_swr_sample_rate;
	av_channel_layout_copy(&dstFrame->ch_layout, &m_swr_ch_layout);

	if (0 > av_frame_get_buffer(dstFrame, 0))
	{
		av_frame_free(&dstFrame);
		return nullptr;
	}

	if (0 > swr_convert(m_pSwrCtx, dstFrame->data, dstFrame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples))
	{
		av_frame_free(&dstFrame);
		return nullptr;
	}
	dstFrame->pts = frame->pts;
	dstFrame->best_effort_timestamp = frame->best_effort_timestamp;

	return dstFrame;
}
