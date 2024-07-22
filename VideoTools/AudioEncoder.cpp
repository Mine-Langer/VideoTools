#include "AudioEncoder.h"

CAudioEncoder::CAudioEncoder()
{

}

CAudioEncoder::~CAudioEncoder()
{
	Cleanup();
}

CAudioEncoder* CAudioEncoder::CreateObject()
{
	return new CAudioEncoder();
}

void CAudioEncoder::Close()
{
	Cleanup();
}

AVCodecContext* CAudioEncoder::Init(AVCodecID CodecId, AVSampleFormat SampleFmt, int SampleRate, int BitRate)
{
	const AVCodec* pCodec = avcodec_find_encoder(CodecId);
	if (!pCodec)
		return nullptr;

	m_pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!m_pCodecCtx)
		return nullptr;

	m_pCodecCtx->codec_id = CodecId;
	av_channel_layout_default(&m_pCodecCtx->ch_layout, 2);
	m_pCodecCtx->sample_fmt = pCodec->sample_fmts[0];
	m_pCodecCtx->sample_rate = SampleRate;
	m_pCodecCtx->bit_rate = BitRate;
	m_pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	m_pCodecCtx->time_base = { 1, m_pCodecCtx->sample_rate };

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return nullptr;

	m_pAudioFifo = av_audio_fifo_alloc(m_pCodecCtx->sample_fmt, m_pCodecCtx->ch_layout.nb_channels, 1);
	if (!m_pAudioFifo)
		return nullptr;

	m_nbSamples = m_pCodecCtx->frame_size ? m_pCodecCtx->frame_size : 1024;

	return m_pCodecCtx;
}

void CAudioEncoder::Start(IRemuxEvent* pEvt)
{
	m_pRemuxEvt = pEvt;

	m_bRun = true;
	m_thread = std::thread(&CAudioEncoder::Work, this);
}

bool CAudioEncoder::SetResampler()
{
	if (m_pSwrCtx)
		return true;

	m_pSwrCtx = swr_alloc();
// 	av_opt_set_int(m_pSwrCtx, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
// 	av_opt_set_int(m_pSwrCtx, "in_sample_rate", 44100, 0);
// 	av_opt_set_sample_fmt(m_pSwrCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
// 	av_opt_set_int(m_pSwrCtx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
// 	av_opt_set_int(m_pSwrCtx, "out_sample_rate", 44100, 0);
// 	av_opt_set_sample_fmt(m_pSwrCtx, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
	swr_alloc_set_opts2(&m_pSwrCtx,
		&m_pCodecCtx->ch_layout,
		m_pCodecCtx->sample_fmt,
		m_pCodecCtx->sample_rate,
		&m_pCodecCtx->ch_layout,
		AV_SAMPLE_FMT_S16,
		m_pCodecCtx->sample_rate,
		0, nullptr);

	if (0 > swr_init(m_pSwrCtx))
		return false;
	
	return true;
}

void CAudioEncoder::PushFrameToFIFO(CAVFrame* frame)
{
	return;
}

void CAudioEncoder::PushFrameToFIFO(AVFrame* frame)
{
	if (frame == nullptr)
	{
		m_bFinished = true;
		return;
	}

	std::unique_lock<std::mutex> lock(m_audioMutex);
	int frame_size = frame->nb_samples;
	int fifo_size = av_audio_fifo_size(m_pAudioFifo);

	int err = av_audio_fifo_realloc(m_pAudioFifo, fifo_size + frame_size);
	
	int wsize = av_audio_fifo_write(m_pAudioFifo, (void**)frame->data, frame_size);
}

void CAudioEncoder::PushFrameToFIFO(uint8_t* pBuf, int BufSize)
{
	std::unique_lock<std::mutex> lock(m_audioMutex);
	int src_nb_samples = BufSize;
	int dst_nb_samples = av_rescale_rnd(swr_get_delay(m_pSwrCtx, 44100) + src_nb_samples, 44100, 44100, AV_ROUND_UP);
	uint8_t** dst_data = nullptr;
	av_samples_alloc_array_and_samples(&dst_data, nullptr, 2, dst_nb_samples, AV_SAMPLE_FMT_FLTP, 0);

	int ret = swr_convert(m_pSwrCtx, dst_data, dst_nb_samples, (const uint8_t**)&pBuf, src_nb_samples);
	if (ret < 0) {

	}
	int fifo_size = av_audio_fifo_size(m_pAudioFifo);

	int err = av_audio_fifo_realloc(m_pAudioFifo, fifo_size + ret);

	int wsize = av_audio_fifo_write(m_pAudioFifo, (void**)dst_data, ret);

	av_freep(&dst_data[0]);
	av_freep(&dst_data);
}

double CAudioEncoder::GetTimebase() const
{
	return av_q2d(m_pCodecCtx->time_base);
}

void CAudioEncoder::Work()
{
	int fifo_size = 0;
	int ret = 0;

	while (m_bRun)
	{
		fifo_size = av_audio_fifo_size(m_pAudioFifo);
		if (fifo_size >= m_nbSamples || (fifo_size >= 0 && m_bFinished))
		{
			if (fifo_size == 0 && m_bFinished)
			{
				m_bRun = false;
				break;
			}

			const int frameSize = FFMIN(av_audio_fifo_size(m_pAudioFifo), m_nbSamples);
			AVFrame* outputFrame = AllocOutputFrame(frameSize);
			
			{
				std::unique_lock<std::mutex> lock(m_audioMutex);
				ret = av_audio_fifo_read(m_pAudioFifo, (void**)outputFrame->data, frameSize);
				if (ret < frameSize)
				{
					char errbuf[128];
					av_strerror(ret, errbuf, sizeof(errbuf));
					fprintf(stderr, "Error reading from FIFO: %s\n", errbuf);
					continue;
				}
			}

			outputFrame->pts = m_frameIndex;
			m_frameIndex += outputFrame->nb_samples;

			ret = avcodec_send_frame(m_pCodecCtx, outputFrame);
			if (ret < 0)
			{
				av_frame_free(&outputFrame);
				continue;
			}
			av_frame_free(&outputFrame);

			AVPacket* pkt = av_packet_alloc();
			ret = avcodec_receive_packet(m_pCodecCtx, pkt);
			if (ret < 0)
			{
				continue;
			}
			//double q2d = av_q2d(m_pCodecCtx->time_base);
			//int realtime = pkt->pts * q2d;
			m_pRemuxEvt->RemuxEvent(pkt, AVMEDIA_TYPE_AUDIO, m_frameIndex);

			//printf("encode audio -> pts:%lld  q2d:%f realtime:%d  framesize:%d\r\n", pkt->pts, q2d, realtime, frameSize);

			if (fifo_size >= 0 && fifo_size < m_nbSamples && m_bFinished)
			{
				m_bRun = false;
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	m_pRemuxEvt->RemuxEvent(nullptr, AVMEDIA_TYPE_AUDIO, m_frameIndex); // ½áÊø±êÖ¾
	printf("Audio Encode Finished....\r\n");
}

AVFrame* CAudioEncoder::AllocOutputFrame(int nbSize)
{
	AVFrame* outputFrame = av_frame_alloc();
	if (!outputFrame)
		return nullptr;

	outputFrame->nb_samples = nbSize;
	outputFrame->ch_layout = m_pCodecCtx->ch_layout;
	outputFrame->format = m_pCodecCtx->sample_fmt;
	outputFrame->sample_rate = m_pCodecCtx->sample_rate;

	if (0 > av_frame_get_buffer(outputFrame, 0)) {
		av_frame_free(&outputFrame);
		return nullptr;
	}

	return outputFrame;
}

void CAudioEncoder::Cleanup()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_pAudioFifo)
	{
		av_audio_fifo_free(m_pAudioFifo);
		m_pAudioFifo = nullptr;
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
