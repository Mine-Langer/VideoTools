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

bool CAudioDecoder::SendPacket(AVPacket* pkt)
{
	if (pkt == nullptr)
		m_srcAPktQueue.MaxSizePush(pkt, &m_bRun);
	else
	{
		AVPacket* tpkt = av_packet_clone(pkt);
		if (!tpkt)
			return false;

		m_srcAPktQueue.MaxSizePush(tpkt, &m_bRun);
	}
	
	return true;
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

	while (!m_srcAPktQueue.Empty())
	{
		AVPacket* pkt = nullptr;
		m_srcAPktQueue.Pop(pkt);
		if (pkt)
			av_packet_free(&pkt);
	}
}

void CAudioDecoder::GetSrcParameter(int& sample_rate, AVChannelLayout& ch_layout, enum AVSampleFormat& sample_fmt)
{
	sample_rate = m_pCodecCtx->sample_rate;
	ch_layout = m_pCodecCtx->ch_layout;
	sample_fmt = m_pCodecCtx->sample_fmt;
}

bool CAudioDecoder::SetSwrContext(AVChannelLayout ch_layout, enum AVSampleFormat sample_fmt, int sample_rate)
{
	m_sampleFormat = sample_fmt;
	m_sampleRate = sample_rate;

	if (0 > swr_alloc_set_opts2(&m_pSwrCtx, &ch_layout, m_sampleFormat, m_sampleRate,
		&m_pCodecCtx->ch_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr))
		return false;

	if (0 > swr_init(m_pSwrCtx))
		return false;

	m_nbSamples = av_rescale_rnd(swr_get_delay(m_pSwrCtx, m_pCodecCtx->sample_rate) + m_pCodecCtx->frame_size,
		m_sampleRate, m_pCodecCtx->sample_rate, AV_ROUND_INF);

	return true;
}


AVChannelLayout CAudioDecoder::GetChannelLayout()
{
	return m_pCodecCtx->ch_layout;
}

int64_t CAudioDecoder::Timebase()
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
					m_pEvent->AudioEvent(nullptr);
					break;
				}
				else
				{
					error = avcodec_send_packet(m_pCodecCtx, pkt);
					if (error < 0)
						continue;

					error = avcodec_receive_frame(m_pCodecCtx, srcFrame);
					if (error < 0)
					{
						continue;
					}

					AVFrame* cvtFrame = ConvertFrame(srcFrame);

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
	if (0 > av_samples_alloc(dstFrame->data, dstFrame->linesize,
		m_pCodecCtx->ch_layout.nb_channels, m_nbSamples, m_sampleFormat, 1))
	{
		av_frame_free(&dstFrame);
		return nullptr;
	}

	if (0 > swr_convert(m_pSwrCtx, dstFrame->data, m_nbSamples, (const uint8_t**)frame->data, frame->nb_samples))
	{
		av_frame_free(&dstFrame);
		return nullptr;
	}

	return dstFrame;
}

/*bool CAudioDecoder::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_AUDIO)
	{
		bool bRun = (m_state != Stopped);
		if (pkt)
		{
			AVPacket* packet = av_packet_clone(pkt);
			m_srcAPktQueue.MaxSizePush(packet, &bRun);
		}
		else
		{
			m_srcAPktQueue.MaxSizePush(nullptr, &bRun);
		}
	}
	return true;
}*/

