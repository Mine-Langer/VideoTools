#include "AudioEncoder.h"

CAudioEncoder::CAudioEncoder()
{

}

CAudioEncoder::~CAudioEncoder()
{

}

bool CAudioEncoder::InitAudio(AVFormatContext* formatCtx, AVCodecID codecId, 
	AVSampleFormat sampleFmt, int sampleRate, int bitRate)
{
	const AVCodec* pCodec = avcodec_find_encoder(codecId);
	if (pCodec == nullptr)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(pCodec);
	if (m_pCodecCtx == nullptr)
		return false;

	m_pCodecCtx->codec_id = codecId;
	av_channel_layout_default(&m_pCodecCtx->ch_layout, 2);
	m_pCodecCtx->sample_fmt = sampleFmt;
	m_pCodecCtx->sample_rate = sampleRate;
	m_pCodecCtx->bit_rate = bitRate;
	m_pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	m_pCodecCtx->time_base = { 1, m_pCodecCtx->sample_rate };
	
	m_pStream = avformat_new_stream(formatCtx, nullptr);
	m_pStream->time_base = m_pCodecCtx->time_base;
	m_pStream->id = formatCtx->nb_streams - 1;

	if (formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		m_pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	if (0 > avcodec_parameters_from_context(m_pStream->codecpar, m_pCodecCtx))
		return false;

	m_pAudioFifo = av_audio_fifo_alloc(m_pCodecCtx->sample_fmt, m_pCodecCtx->ch_layout.nb_channels, 1);
	if (m_pAudioFifo == nullptr)
		return false;

	m_nbSamples = m_pCodecCtx->frame_size;
	if (!m_nbSamples)
		m_nbSamples = 1024;

	return true;
}

void CAudioEncoder::Start(IEncoderEvent* pEvt)
{
	m_pEvent = pEvt;

	m_bRun = true;
	m_thread = std::thread(&CAudioEncoder::OnWork, this);
}


void CAudioEncoder::Release()
{
	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (m_pAudioFifo) {
		av_audio_fifo_free(m_pAudioFifo);
		m_pAudioFifo = nullptr;
	}
}


AVRational CAudioEncoder::GetTimeBase()
{
	return m_pCodecCtx->time_base;
}

bool CAudioEncoder::PushFrame(AVFrame* frame)
{
	m_pAudioQueue.MaxSizePush(frame, &m_bRun);

	return false;
}

bool CAudioEncoder::PushFrameToFifo(AVFrame* frameData)
{
	int framesize = frameData->nb_samples;
	int fifoSize = av_audio_fifo_size(m_pAudioFifo);

	int err = av_audio_fifo_realloc(m_pAudioFifo, fifoSize + framesize);

	int wsize = av_audio_fifo_write(m_pAudioFifo, (void**)frameData->data, framesize);
	
	av_frame_free(&frameData);

	return true;
}

int CAudioEncoder::GetIndex()
{
	return m_frameIndex;
}

void CAudioEncoder::OnWork()
{
	AVFrame* frame = nullptr;
	while (m_bRun)
	{
		if (!m_pAudioQueue.Pop(frame))
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		else
		{
			if (frame == nullptr) {
				break;
			}
			else
			{
				PushFrameToFifo(frame);

				ReadPacketFromFifo();
			}
		}
	}
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
bool CAudioEncoder::ReadPacketFromFifo()
{
	int fifo_size = 0;
	int error = 0;
	while ((fifo_size = av_audio_fifo_size(m_pAudioFifo) >= m_nbSamples) || (m_bFinished && fifo_size > 0))
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

		AVPacket* pkt = av_packet_alloc();
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

		m_pEvent->AudioEvent(pkt, m_frameIndex);

		int realtime = pkt->pts * av_q2d(m_pCodecCtx->time_base);
		printf("pts:%lld  realtime:%d  framesize:%d\r\n", pkt->pts, realtime, frameSize);

		av_frame_free(&outputFrame);
		return true;
	}
	return false;
}

