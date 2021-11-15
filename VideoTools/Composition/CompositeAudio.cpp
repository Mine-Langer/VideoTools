#include "CompositeAudio.h"

CompositeAudio::CompositeAudio()
{

}

CompositeAudio::~CompositeAudio()
{

}

bool CompositeAudio::Open(const char* szInput)
{
	if (!m_demux.Open(szInput))
		return false;

	AVStream* pStream = m_demux.FormatContext()->streams[m_demux.AudioStreamIndex()];
	if (!pStream)
		return false;

	m_codecCtx = avcodec_alloc_context3(nullptr);
	if (!m_codecCtx)
		return false;

	if (0 > avcodec_parameters_to_context(m_codecCtx, pStream->codecpar))
		return false;

	AVCodec* codec = avcodec_find_decoder(m_codecCtx->codec_id);
	if (!codec)
		return false;

	if (0 > avcodec_open2(m_codecCtx, codec, nullptr))
		return false;

	return true;
}

void CompositeAudio::GetSrcParameter(int& sample_rate, int& nb_sample, int64_t& ch_layout, enum AVSampleFormat& sample_fmt)
{
	sample_rate = m_codecCtx->sample_rate;
	nb_sample = m_codecCtx->frame_size;
	ch_layout = m_codecCtx->channel_layout;
	sample_fmt = m_codecCtx->sample_fmt;
}

bool CompositeAudio::SetSwrContext(int64_t ch_layout, enum AVSampleFormat sample_fmt, int sample_rate)
{
	m_ch_layout = ch_layout;
	m_sample_fmt = sample_fmt;
	m_sample_rate = sample_rate;

	m_swrCtx = swr_alloc_set_opts(nullptr, m_ch_layout, m_sample_fmt, sample_rate,
		m_codecCtx->channel_layout, m_codecCtx->sample_fmt, m_codecCtx->sample_rate, 0, nullptr);
	if (!m_swrCtx)
		return false;

	if (0 > swr_init(m_swrCtx))
		return false;

	m_nb_samples = av_rescale_rnd(swr_get_delay(m_swrCtx, m_codecCtx->sample_rate)+m_codecCtx->frame_size, 
		m_sample_rate, m_codecCtx->sample_rate, AV_ROUND_INF);

	return true;
}

void CompositeAudio::Start(IAudioEvent* pEvt)
{
	m_pEvent = pEvt;
	m_demux.Start(this);

	m_bRun = true;
	m_thread = std::thread(&CompositeAudio::OnDecodeFunction, this);
}

void CompositeAudio::Release()
{
	m_demux.Release();
}

void CompositeAudio::OnDecodeFunction()
{
	int ret = 0;

	while (m_bRun)
	{
		if (m_audioQueue.Empty())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		else
		{
			AVPacket* pkt = nullptr;
			m_audioQueue.Pop(pkt);

			if (0 > avcodec_send_packet(m_codecCtx, pkt))
			{
				av_packet_free(&pkt);
				continue;
			}

			AVFrame* frame = av_frame_alloc();
			if (0 != avcodec_receive_frame(m_codecCtx, frame))
			{
				av_packet_free(&pkt);
				continue;
			}
			// зЊТы
// 			AVFrame* dstFrame = av_frame_alloc();
// 			if (0 > av_samples_alloc(dstFrame->data, dstFrame->linesize, 
// 				av_get_channel_layout_nb_channels(m_ch_layout), m_nb_samples, m_sample_fmt, 1))
// 			{
// 				av_frame_unref(frame);
// 				av_packet_free(&pkt);
// 				continue;
// 			}
// 
// 			swr_convert(m_swrCtx, dstFrame->data, m_nb_samples, (const uint8_t**)frame->data, frame->nb_samples);

			m_pEvent->AudioEvent(frame);

		//	av_frame_unref(frame);
			av_packet_free(&pkt);
		}
	}

}

bool CompositeAudio::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_AUDIO)
	{
		AVPacket* packet = av_packet_clone(pkt);
		m_audioQueue.Push(packet);
	}

	return true;
}
