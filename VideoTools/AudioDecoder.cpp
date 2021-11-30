#include "AudioDecoder.h"

CAudioDecoder::CAudioDecoder()
{

}

CAudioDecoder::~CAudioDecoder()
{

}

bool CAudioDecoder::Open(const char* szInput)
{
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

	AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	return true;
}

bool CAudioDecoder::OpenMicrophone(const char* szUrl)
{
	AVInputFormat* ifmt = av_find_input_format("dshow");
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

	AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	return true;
}

bool CAudioDecoder::Start(IAudioEvent* pEvt)
{
	m_pEvent = pEvt;
	if (!m_pEvent)
		return false;

	m_demux.Start(this);

	m_state = Started;
	m_thread = std::thread(&CAudioDecoder::OnDecodeFunction, this);
	m_thread.detach();
	return false;
}

void CAudioDecoder::GetSrcParameter(int& sample_rate, int& nb_sample, int64_t& ch_layout, enum AVSampleFormat& sample_fmt)
{
	sample_rate = m_pCodecCtx->sample_rate;
	nb_sample = m_pCodecCtx->frame_size;
	ch_layout = m_pCodecCtx->channel_layout;
	sample_fmt = m_pCodecCtx->sample_fmt;
}

bool CAudioDecoder::SetSwrContext(int64_t ch_layout, enum AVSampleFormat sample_fmt, int sample_rate)
{
	m_channelLayout = ch_layout;
	m_sampleFormat = sample_fmt;
	m_sampleRate = sample_rate;

	m_pSwrCtx = swr_alloc_set_opts(nullptr, m_channelLayout, m_sampleFormat, m_sampleRate,
		m_pCodecCtx->channel_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr);
	if (!m_pSwrCtx)
		return false;

	if (0 > swr_init(m_pSwrCtx))
		return false;

	m_nbSamples = av_rescale_rnd(swr_get_delay(m_pSwrCtx, m_pCodecCtx->sample_rate) + m_pCodecCtx->frame_size,
		m_sampleRate, m_pCodecCtx->sample_rate, AV_ROUND_INF);

	return true;
}

void CAudioDecoder::SetSaveEnable(bool isSave)
{
	m_bIsSave = isSave;
}

void CAudioDecoder::OnDecodeFunction()
{
	int error = 0;
	AVPacket* pkt = nullptr;
	AVFrame* srcFrame = nullptr;
	if (!m_bIsSave)
		srcFrame = av_frame_alloc();

	while (m_state != Stopped)
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
					// 结束标志
					m_pEvent->AudioEvent(nullptr);
					break;
				}
				else
				{
					error = avcodec_send_packet(m_pCodecCtx, pkt);
					if (error < 0)
						continue;

					if (m_bIsSave)
						srcFrame = av_frame_alloc();

					error = avcodec_receive_frame(m_pCodecCtx, srcFrame);
					if (error < 0)
					{
						if (m_bIsSave)
							av_frame_free(&srcFrame);
						continue;
					}

					// 转码(播放用)
					if (!m_bIsSave)
					{
						AVFrame* dstFrame = ConvertFrame(srcFrame);
						m_pEvent->AudioEvent(dstFrame);
					}
					else
					{
						m_pEvent->AudioEvent(srcFrame);
					}
					if (!m_bIsSave)
						av_frame_unref(srcFrame);
				}
			}
		}
	}
	if (!m_bIsSave)
		av_frame_free(&srcFrame);
}

AVFrame* CAudioDecoder::ConvertFrame(AVFrame* frame)
{
	AVFrame* dstFrame = av_frame_alloc();
	if (0 > av_samples_alloc(dstFrame->data, dstFrame->linesize,
		av_get_channel_layout_nb_channels(m_channelLayout), m_nbSamples, m_sampleFormat, 1))
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

bool CAudioDecoder::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_AUDIO)
	{
		if (pkt)
		{
			AVPacket* packet = av_packet_clone(pkt);
			m_srcAPktQueue.MaxSizePush(packet);
		}
		else
		{
			m_srcAPktQueue.MaxSizePush(nullptr);
		}
	}
	return true;
}

