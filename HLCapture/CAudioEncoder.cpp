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
	AudioCodecCtx->sample_fmt = pCodec->sample_fmts[0];
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

	AudioFIFO = av_audio_fifo_alloc(AudioCodecCtx->sample_fmt, AudioCodecCtx->channels, 1);
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

}
