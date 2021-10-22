#include "CAudioDecoder.h"

CAudioDecoder::CAudioDecoder()
{

}

CAudioDecoder::~CAudioDecoder()
{

}

bool CAudioDecoder::Open(AVStream* pStream)
{
	AudioCodecCtx = avcodec_alloc_context3(nullptr);
	if (AudioCodecCtx == nullptr)
		return false;

	if (0 > avcodec_parameters_to_context(AudioCodecCtx, pStream->codecpar))
		return false;

	AVCodec* codec = avcodec_find_decoder(AudioCodecCtx->codec_id);
	if (codec == nullptr)
		return false;

	if (0 > avcodec_open2(AudioCodecCtx, codec, nullptr))
		return false;

	m_timebase = av_q2d(pStream->time_base);
	m_duration = m_timebase * (pStream->duration * 1.0);
	m_rate = AudioCodecCtx->sample_rate;

	SrcFrame = av_frame_alloc();

	return true;
}

void CAudioDecoder::Start(IDecoderEvent* evt)
{
	m_event = evt;
	m_bRun = true;
	m_decodeThread = std::thread(&CAudioDecoder::OnDecodeFunction, this);
}

bool CAudioDecoder::SendPacket(AVPacket* pkt)
{
	AVPacket* apkt = av_packet_clone(pkt);
	m_packetQueue.MaxSizePush(apkt);
	
	return true;
}

bool CAudioDecoder::SetConfig()
{
	m_channel_layout = AudioCodecCtx->channel_layout;
	m_sample_fmt = AV_SAMPLE_FMT_S16;
	m_sample_rate = AudioCodecCtx->sample_rate;

	SwrCtx = swr_alloc_set_opts(nullptr, m_channel_layout, m_sample_fmt, m_sample_rate,
		AudioCodecCtx->channel_layout, AudioCodecCtx->sample_fmt, AudioCodecCtx->sample_rate, 0, nullptr);
	if (SwrCtx == nullptr)
		return false;

	if (0 > swr_init(SwrCtx))
		return false;

	m_nb_samples = av_rescale_rnd(swr_get_delay(SwrCtx, AudioCodecCtx->sample_rate) + AudioCodecCtx->frame_size,
		m_sample_rate, AudioCodecCtx->sample_rate, AV_ROUND_INF);

	return true;
}

void CAudioDecoder::Close()
{

}

int CAudioDecoder::GetSampleRate()
{
	return m_sample_rate;
}

int CAudioDecoder::GetChannels()
{
	return av_get_channel_layout_nb_channels(m_channel_layout);
}

int CAudioDecoder::GetSamples()
{
	return m_nb_samples;
}

int CAudioDecoder::GetChannelLayouts()
{
	return m_channel_layout;
}

AVSampleFormat CAudioDecoder::GetSampleFormat()
{
	return AudioCodecCtx->sample_fmt;
}

void CAudioDecoder::OnDecodeFunction()
{
	int error = 0;
	AVPacket* apkt = nullptr;

	while (m_bRun)
	{
		if (!m_packetQueue.Pop(apkt))
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			if (0 > avcodec_send_packet(AudioCodecCtx, apkt))
				break;

			error = avcodec_receive_frame(AudioCodecCtx, SrcFrame);
			if (error < 0) {
				if (error == AVERROR(EAGAIN) || error == AVERROR_EOF)
					continue;
				break;
			}
			else if (error == 0)
			{
				STAudioBuffer* audioBuf = new STAudioBuffer();
				if (0 > av_samples_alloc(&audioBuf->buffer, &audioBuf->size, av_get_channel_layout_nb_channels(m_channel_layout), m_nb_samples, m_sample_fmt, 1))
					break;

				swr_convert(SwrCtx, &audioBuf->buffer, m_nb_samples, (const uint8_t**)SrcFrame->data, SrcFrame->nb_samples);
				audioBuf->pts = SrcFrame->best_effort_timestamp;
				audioBuf->dpts = audioBuf->pts * m_timebase;
				audioBuf->duration = 1.0 / m_rate;
				m_event->AudioEvent(audioBuf);
				
				//av_frame_free(&dstFrame);
			}
			av_frame_unref(SrcFrame);
		}
		av_packet_free(&apkt);
	}
}
