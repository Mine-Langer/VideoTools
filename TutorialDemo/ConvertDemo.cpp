#include "ConvertDemo.h"

CAudioFrame::~CAudioFrame()
{
	Release();
}

bool CAudioFrame::Open(const char* szfile, int begin, int end)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szfile, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	int audio_idx = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (0 > audio_idx)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (0 > avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[audio_idx]->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_begin_pts = begin * AV_TIME_BASE;
	m_end_pts = end * AV_TIME_BASE;

	m_timebase = av_q2d(m_pCodecCtx->time_base);
	m_audio_idx = audio_idx;

	m_bRun = true;
	m_tRead = std::thread(&CAudioFrame::OnRun, this);

	return true;
}

AVFrame* CAudioFrame::AudioFrame(bool& bstatus)
{
	AVFrame* frame = nullptr;
	bstatus = m_frameQueueData.Pop(frame);
	return frame;
}

void CAudioFrame::Release()
{
	if (m_tRead.joinable())
		m_tRead.join();

	if (m_pFormatCtx) {
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}

	if (m_pCodecCtx) {
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}
}

void CAudioFrame::OnRun()
{
	if (0 > avformat_seek_file(m_pFormatCtx, -1, INT_MIN, m_begin_pts, INT_MAX, 0))
		return;

	AVPacket pkt;
	while (m_bRun)
	{
		if (0 > av_read_frame(m_pFormatCtx, &pkt))
		{
			m_frameQueueData.MaxSizePush(nullptr, &m_bRun);
			break;
		}

		if (pkt.stream_index != m_audio_idx)
			continue;

		if (0 > avcodec_send_packet(m_pCodecCtx, &pkt))
			continue;

		AVFrame* frame = av_frame_alloc();
		if (0 > avcodec_receive_frame(m_pCodecCtx, frame))
		{
			av_frame_free(&frame);
			continue;
		}

		m_frameQueueData.MaxSizePush(frame, &m_bRun);
	}
}

//////////////////////////////////////////////////////////////////////////////////

bool AEncode::Open(AVFormatContext* fmt_ctx, AVCodecID codec_id, int src_samples, AVChannelLayout src_ch_layout, AVSampleFormat src_fmt)
{

	return false;
}



//////////////////////////////////////////////////////////////////////////////////
bool ConvertDemo::Save(const char* szOut, const char* szInput)
{
	OpenOutput(szOut);

	audioFrame.Open(szInput);

	m_thread = std::thread(&ConvertDemo::OnThreadFunc, this);
	return false;
}

void ConvertDemo::OpenOutput(const char* szOut)
{
	if (0 > avformat_alloc_output_context2(&m_pFormatCtx, nullptr, nullptr, szOut))
		return;

	AVCodecID codec_id_a = m_pFormatCtx->oformat->audio_codec;
	audioEncode.Open(m_pFormatCtx, codec_id_a, );
}

void ConvertDemo::Release()
{
}

void ConvertDemo::OnThreadFunc()
{

}
