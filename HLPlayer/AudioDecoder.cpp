#include "AudioDecoder.h"

CAudioDecoder::CAudioDecoder()
{

}

CAudioDecoder::~CAudioDecoder()
{

}

bool CAudioDecoder::Open(AVStream* pStream)
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

	m_pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	m_pCodecCtx->channels = av_get_channel_layout_nb_channels(m_pCodecCtx->channel_layout);

	SetConfig();

	return true;
}

void CAudioDecoder::Start(IDecoderEvent* pEvent)
{
	m_pEvent = pEvent;

	m_status = ePlay;
	m_decodeThread = std::thread(&CAudioDecoder::OnDecodeThread, this);
}

void CAudioDecoder::Close()
{
	m_status = eStop;
	if (m_decodeThread.joinable())
		m_decodeThread.join();

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

	while (!m_audioQueue.Empty())
	{
		AVPacket* pkt = nullptr;
		m_audioQueue.Pop(pkt);
		if (pkt)
			av_packet_free(&pkt);
	}
}

void CAudioDecoder::PushPacket(AVPacket* pkt)
{
	AVPacket* apkt = nullptr;
	if (pkt)
		apkt = av_packet_clone(pkt);

	bool bRun = (m_status == eStop);
	m_audioQueue.MaxSizePush(apkt, &bRun);
}

void CAudioDecoder::OnDecodeThread()
{
	AVPacket* packet = nullptr;
	AVFrame* frame = av_frame_alloc();
	int err = 0;

	while (m_status != eStop)
	{
		if (m_status == ePause)
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			if (!m_audioQueue.Pop(packet))
				std::this_thread::sleep_for(std::chrono::milliseconds(40));
			else
			{
				if (packet == nullptr)
				{
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

					err = avcodec_receive_frame(m_pCodecCtx, frame);
					if (err < 0)
					{
						av_packet_free(&packet);
						continue;
					}

					AVFrame* pSwrFrame = ConvertFrame(frame);

					av_frame_unref(frame);

					m_pEvent->AudioEvent(pSwrFrame);
					
					av_packet_free(&packet);
				}
			}
		}
	}

	av_frame_free(&frame);
}

bool CAudioDecoder::SetConfig()
{
	m_channelLayout = AV_CH_LAYOUT_STEREO;
	m_sampleFmt = AV_SAMPLE_FMT_S16;
	m_sampleRate = m_pCodecCtx->sample_rate;

	m_pSwrCtx = swr_alloc_set_opts(nullptr, m_channelLayout, m_sampleFmt, m_sampleRate,
		m_pCodecCtx->channel_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr);
	if (!m_pSwrCtx)
		return false;

	if (0 > swr_init(m_pSwrCtx))
		return false;

	m_nbSamples = av_rescale_rnd(swr_get_delay(m_pSwrCtx, m_pCodecCtx->sample_rate)+m_pCodecCtx->frame_size,
		m_sampleRate, m_pCodecCtx->sample_rate, AV_ROUND_INF);

	return true;
}

AVFrame* CAudioDecoder::ConvertFrame(AVFrame* frame)
{
	AVFrame* swrFrame = av_frame_alloc();
	if (0 > av_samples_alloc(&swrFrame->data[0], &swrFrame->nb_samples, av_get_channel_layout_nb_channels(m_channelLayout), m_nbSamples, m_sampleFmt, 1))
	{
		av_frame_free(&swrFrame);
		return nullptr;
	}
	
	if (0 > swr_convert(m_pSwrCtx, swrFrame->data, m_nbSamples, (const uint8_t**)frame->data, frame->nb_samples))
	{
		av_frame_free(&swrFrame);
		return nullptr;
	}
	
	swrFrame->pts = frame->pts;
	swrFrame->pkt_dts = swrFrame->pts * m_timebase;
	
	return swrFrame;
}

void CAudioDecoder::InitAudioSpec(SDL_AudioSpec& audioSpec)
{
	audioSpec.freq = m_pCodecCtx->sample_rate;
	audioSpec.channels = m_pCodecCtx->channels;
	audioSpec.samples = m_nbSamples;
}
