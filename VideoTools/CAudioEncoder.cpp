#include "CAudioEncoder.h"

CAudioEncoder::CAudioEncoder()
{

}

CAudioEncoder::~CAudioEncoder()
{

}

bool CAudioEncoder::InitAudio(AVFormatContext* formatCtx, AVCodecID codecId, CAudioDecoder* audioDecoder)
{
	AVCodec* pCodec = avcodec_find_encoder(codecId);
	if (pCodec == nullptr)
		return false;

	AudioCodecCtx = avcodec_alloc_context3(pCodec);
	if (AudioCodecCtx == nullptr)
		return false;

	AudioCodecCtx->codec_id = codecId;
	AudioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	AudioCodecCtx->channels = av_get_channel_layout_nb_channels(AudioCodecCtx->channel_layout);
	AudioCodecCtx->sample_fmt = pCodec->sample_fmts ? pCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	AudioCodecCtx->sample_rate = 44100;
	AudioCodecCtx->bit_rate = 96000;
	AudioCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	AudioCodecCtx->time_base = { 1, AudioCodecCtx->sample_rate };
	
	AudioStream = avformat_new_stream(formatCtx, nullptr);
	AudioStream->time_base = AudioCodecCtx->time_base;

	if (formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		AudioCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (0 > avcodec_open2(AudioCodecCtx, pCodec, nullptr))
		return false;

	avcodec_parameters_from_context(AudioStream->codecpar, AudioCodecCtx);

	SwrCtx = swr_alloc_set_opts(nullptr, AudioCodecCtx->channel_layout, AudioCodecCtx->sample_fmt, AudioCodecCtx->sample_rate,
		audioDecoder->GetChannelLayouts(), audioDecoder->GetSampleFormat(), audioDecoder->GetSampleRate(), 0, nullptr);
	if (SwrCtx == nullptr)
		return false;

	if (0 > swr_init(SwrCtx))
		return false;

	m_nbSamples = AudioCodecCtx->frame_size;
	if (!m_nbSamples)
		m_nbSamples = 1024;

	AudioFIFO = av_audio_fifo_alloc(AudioCodecCtx->sample_fmt, AudioCodecCtx->channels, m_nbSamples);
	if (AudioFIFO == nullptr)
		return false;

	return true;
}

void CAudioEncoder::Start(IEncoderEvent* pEvt)
{
	m_event = pEvt;
	
	m_bRun = true;
	m_encodeThread = std::thread(&CAudioEncoder::OnEncodeThread, this);
}

void CAudioEncoder::PushFrame(AVFrame* frame)
{
	m_audioFrameQueue.Push(frame);
}

void CAudioEncoder::Release()
{
	if (AudioCodecCtx)
	{
		avcodec_close(AudioCodecCtx);
		avcodec_free_context(&AudioCodecCtx);
		AudioCodecCtx = nullptr;
	}
}

void CAudioEncoder::OnEncodeThread()
{
	int error = 0;
	while (m_bRun)
	{
		if (m_audioFrameQueue.Size() == 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		else
		{
			ConvertAudioBuffer();

			GetConvertBuffer();
		}
	}
}

void CAudioEncoder::ConvertAudioBuffer()
{
	int frameSize = AudioCodecCtx->frame_size;
	while (av_audio_fifo_size(AudioFIFO) < frameSize)
	{
		AVFrame* audioFrame = nullptr;
		m_audioFrameQueue.Pop(audioFrame);
		uint8_t** convertBuffer = new uint8_t* [AudioCodecCtx->channels]();
		av_samples_alloc(convertBuffer, nullptr, AudioCodecCtx->channels, audioFrame->nb_samples, AudioCodecCtx->sample_fmt, 0);
		swr_convert(SwrCtx, convertBuffer, audioFrame->nb_samples, (const uint8_t**)audioFrame->extended_data, audioFrame->nb_samples);
		av_audio_fifo_realloc(AudioFIFO, av_audio_fifo_size(AudioFIFO) + audioFrame->nb_samples);
		av_audio_fifo_write(AudioFIFO, (void**)convertBuffer, audioFrame->nb_samples);
		av_freep(&convertBuffer[0]);
		delete[] convertBuffer;
		av_frame_free(&audioFrame);
	}
}

void CAudioEncoder::GetConvertBuffer()
{
	const int frameSize = FFMIN(av_audio_fifo_size(AudioFIFO), AudioCodecCtx->frame_size);
	AVFrame* outputFrame = av_frame_alloc();
	outputFrame->nb_samples = frameSize;
	outputFrame->channel_layout = AudioCodecCtx->channel_layout;
	outputFrame->format = AudioCodecCtx->sample_fmt;
	outputFrame->sample_rate = AudioCodecCtx->sample_rate;
	av_frame_get_buffer(outputFrame, 0);

	av_audio_fifo_read(AudioFIFO, (void**)outputFrame->data, frameSize);
	outputFrame->pts = m_pts;
	m_pts += outputFrame->nb_samples;

	EncodeFrame(outputFrame);

	av_frame_free(&outputFrame);
}

void CAudioEncoder::EncodeFrame(AVFrame* frame)
{
	if (0 > avcodec_send_frame(AudioCodecCtx, frame))
		return;

	AVPacket pkt = { 0 };
	int error = avcodec_receive_packet(AudioCodecCtx, &pkt);
	if (error < 0)
	{
		if (error != AVERROR(EAGAIN) && error != AVERROR_EOF)
			return;
	}
	else if (error == 0)
	{
		av_packet_rescale_ts(&pkt, AudioCodecCtx->time_base, AudioStream->time_base);
		pkt.stream_index = AudioStream->index;
		printf(" audio packet pts: %I64d.\r\n", pkt.pts);
		m_event->AudioEvent(&pkt);
	}
	av_packet_unref(&pkt);
}
