#include "AudioEncoder.h"

CAudioEncoder::CAudioEncoder()
{

}

CAudioEncoder::~CAudioEncoder()
{

}

bool CAudioEncoder::InitAudio(AVFormatContext* formatCtx, AVCodecID codecId, AVChannelLayout srcChLayout, enum AVSampleFormat srcSampleFmt, int srcSampleRate)
{
	const AVCodec* pCodec = avcodec_find_encoder(codecId);
	if (pCodec == nullptr)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(pCodec);
	if (m_pCodecCtx == nullptr)
		return false;

	m_pCodecCtx->codec_id = codecId;
	av_channel_layout_default(&m_pCodecCtx->ch_layout, 2);
	m_pCodecCtx->sample_fmt = pCodec->sample_fmts ? pCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	m_pCodecCtx->sample_rate = srcSampleRate;
	m_pCodecCtx->bit_rate = 96000;
	m_pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	m_pCodecCtx->time_base = { 1, m_pCodecCtx->sample_rate };
	
	m_pStream = avformat_new_stream(formatCtx, nullptr);
	m_pStream->time_base = m_pCodecCtx->time_base;

	if (formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		m_pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	if (0 > avcodec_parameters_from_context(m_pStream->codecpar, m_pCodecCtx))
		return false;

	if (0 > swr_alloc_set_opts2(&m_pSwrCtx, &m_pCodecCtx->ch_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate,
		&srcChLayout, srcSampleFmt, srcSampleRate, 0, nullptr))
		return false;

	if (0 > swr_init(m_pSwrCtx))
		return false;

	m_pAudioFifo = av_audio_fifo_alloc(m_pCodecCtx->sample_fmt, m_pCodecCtx->ch_layout.nb_channels, 1);
	if (m_pAudioFifo == nullptr)
		return false;

	m_nbSamples = m_pCodecCtx->frame_size;
	if (!m_nbSamples)
		m_nbSamples = 1024;

	return true;
}

void CAudioEncoder::BindAudioData(SafeQueue<AVFrame*>* audioQueue)
{
	m_pAudioQueue = audioQueue;
}


void CAudioEncoder::Release()
{
	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (m_pSwrCtx) {
		swr_free(&m_pSwrCtx);
		m_pSwrCtx = nullptr;
	}

	if (m_pAudioFifo) {
		av_audio_fifo_free(m_pAudioFifo);
		m_pAudioFifo = nullptr;
	}
}

bool CAudioEncoder::GetEncodePacket(AVPacket* pkt, int& aIndex)
{
	if (!PushAudioToFifo())
		m_bFinished = true;

	while (!ReadPacketFromFifo(pkt))
		continue;
	aIndex = m_frameIndex;

	return !m_bFinished;
}

AVRational CAudioEncoder::GetTimeBase()
{
	return m_pCodecCtx->time_base;
}

bool CAudioEncoder::PushFrameToFifo(const uint8_t** frameData, int framesize)
{
	AVFrame* pFrame = AllocOutputFrame(framesize);
	if (!pFrame)
		return false;
	
	if (0 > swr_convert(m_pSwrCtx, pFrame->data, framesize, frameData, framesize))
		return false;

	int fifoSize = av_audio_fifo_size(m_pAudioFifo);

	int err = av_audio_fifo_realloc(m_pAudioFifo, fifoSize + framesize);

	int wsize = av_audio_fifo_write(m_pAudioFifo, (void**)pFrame->data, framesize);
	
	av_frame_free(&pFrame);

	return true;
}

AVPacket* CAudioEncoder::GetPacketFromFifo(int* aIdx)
{
	int fifosize = av_audio_fifo_size(m_pAudioFifo);
	if (fifosize >= m_pCodecCtx->frame_size)
	{
		const int framesize = FFMIN(fifosize, m_pCodecCtx->frame_size);

		AVFrame* pFrame = AllocOutputFrame(framesize);

		av_audio_fifo_read(m_pAudioFifo, (void**)pFrame->data, framesize);
		pFrame->pts = m_frameIndex;
		m_frameIndex += pFrame->nb_samples;
		*aIdx = m_frameIndex;

		if (0 > avcodec_send_frame(m_pCodecCtx, pFrame)) {
			av_frame_free(&pFrame);
			return nullptr;
		}

		AVPacket* pkt = av_packet_alloc();
		if (0 > avcodec_receive_packet(m_pCodecCtx, pkt)) {
			av_frame_free(&pFrame);
			av_packet_free(&pkt);
			return nullptr;
		}
		av_frame_free(&pFrame);

		av_packet_rescale_ts(pkt, m_pCodecCtx->time_base, m_pStream->time_base);
		pkt->stream_index = m_pStream->index;

		return pkt;
	}
	return nullptr;
}

bool CAudioEncoder::PushAudioToFifo()
{
	AVFrame* frame = nullptr;
	while (av_audio_fifo_size(m_pAudioFifo) < m_nbSamples)
	{
		if (!m_pAudioQueue->Pop(frame))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		if (frame == nullptr)
			return false;

		if (!ReadFrame2Fifo(frame))
			continue;
	}
	return true;
}

bool CAudioEncoder::ReadFrame2Fifo(AVFrame* frame)
{
	bool bRet = false;
	if (!m_convertBuffer)
		m_convertBuffer = new uint8_t* [m_pCodecCtx->channels];
	if (0 > av_samples_alloc(m_convertBuffer, nullptr, m_pCodecCtx->channels, frame->nb_samples, m_pCodecCtx->sample_fmt, 0))
		goto ends;

	if (0 > swr_convert(m_pSwrCtx, m_convertBuffer, frame->nb_samples, (const uint8_t**)frame->extended_data, frame->nb_samples))
		goto ends;

	if (0 > av_audio_fifo_realloc(m_pAudioFifo, av_audio_fifo_size(m_pAudioFifo) + frame->nb_samples))
		goto ends;

	if (0 > av_audio_fifo_write(m_pAudioFifo, (void**)m_convertBuffer, frame->nb_samples))
		goto ends;

	bRet = true;

ends:
	if (m_convertBuffer) {
		av_freep(&m_convertBuffer[0]);
	}
	av_frame_free(&frame);
	return bRet;
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
bool CAudioEncoder::ReadPacketFromFifo(AVPacket* pkt)
{
	int error = 0;
	if (av_audio_fifo_size(m_pAudioFifo) >= m_nbSamples || (m_bFinished && av_audio_fifo_size(m_pAudioFifo) > 0))
	{
		const int frameSize = FFMIN(av_audio_fifo_size(m_pAudioFifo), m_nbSamples);
		AVFrame* outputFrame = AllocOutputFrame(frameSize);
	
		av_audio_fifo_read(m_pAudioFifo, (void**)outputFrame->data, frameSize);
		outputFrame->pts = m_frameIndex;
		m_frameIndex += outputFrame->nb_samples;

		if (0 > avcodec_send_frame(m_pCodecCtx, outputFrame))
		{
			av_frame_free(&outputFrame);
			return false;
		}

		error = avcodec_receive_packet(m_pCodecCtx, pkt);
		if (error < 0)
		{
			bool bRet = false;
			if (error == AVERROR_EOF || error == AVERROR(EAGAIN))
				bRet = true;
			av_frame_free(&outputFrame);
			return bRet;
		}

		av_packet_rescale_ts(pkt, m_pCodecCtx->time_base, m_pStream->time_base);
		pkt->stream_index = m_pStream->index;

		av_frame_free(&outputFrame);
		return true;
	}
	return false;
}

