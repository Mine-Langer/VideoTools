#include "Remultiplexer.h"
#include "AudioEncoder.h"
#include "VideoEncoder.h"

CRemultiplexer::CRemultiplexer()
{
	Init();
}

CRemultiplexer::~CRemultiplexer()
{
	Close();
}

void CRemultiplexer::Init()
{
	m_pAudioEncode = new CAudioEncoder();
	m_pImageEncode = new CVideoEncoder();

	m_AudioEnable = true;
}

void CRemultiplexer::Cleanup()
{
	if (m_pAudioEncode)
	{
		m_pAudioEncode->Close();
	}

	if (m_pImageEncode)
	{
		m_pImageEncode->Close();
	}

	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

bool CRemultiplexer::Open(const std::string& strFile)
{
	if (0 > avformat_alloc_output_context2(&m_pFormatCtx, nullptr, nullptr, strFile.c_str()))
		return false;

	const AVOutputFormat* pOutFormat = m_pFormatCtx->oformat;

	if (m_VideoEnable && pOutFormat->video_codec)
	{
		AVCodecContext* pCodecCtx = m_pImageEncode->Init(pOutFormat->video_codec, m_Width, m_Height);
		if (!pCodecCtx)
			return false;

		m_VideoTimeBase = pCodecCtx->time_base;

		m_pVideoStream = avformat_new_stream(m_pFormatCtx, pCodecCtx->codec);
		if (!m_pVideoStream)
			return false;

		m_pVideoStream->id = m_pFormatCtx->nb_streams - 1;
		avcodec_parameters_from_context(m_pVideoStream->codecpar, pCodecCtx);

		if (pOutFormat->flags & AVFMT_GLOBALHEADER)
			pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	if (m_AudioEnable || pOutFormat->audio_codec)
	{
		AVCodecContext* pCodecCtx = m_pAudioEncode->Init(pOutFormat->audio_codec);
		if (!pCodecCtx)
			return false;

		m_AudioTimeBase = pCodecCtx->time_base;

		m_pAudioStream = avformat_new_stream(m_pFormatCtx, pCodecCtx->codec);
		if (!m_pAudioStream)
			return false;

		m_pAudioStream->id = m_pFormatCtx->nb_streams - 1;
		avcodec_parameters_from_context(m_pAudioStream->codecpar, pCodecCtx);

		if (pOutFormat->flags & AVFMT_GLOBALHEADER)
			pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	if (0 > avio_open(&m_pFormatCtx->pb, strFile.c_str(), AVIO_FLAG_WRITE))
		return false;

	av_dump_format(m_pFormatCtx, 0, nullptr, 1);

	return true;
}


void CRemultiplexer::Start()
{
	m_pAudioEncode->Start(this);

	m_bRun = true;
	m_thread = std::thread(&CRemultiplexer::Work, this);
}

void CRemultiplexer::Close()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();

	while (true)
	{
		AVPacket* pkt = nullptr;
		if (m_AudioPacket.Pop(pkt))
		{
			if (pkt)
				av_packet_free(&pkt);
		}
		else
		{
			break;
		}
	}

	if (m_pAudioEncode)
	{
		delete m_pAudioEncode;
		m_pAudioEncode = nullptr;
	}

	if (m_pImageEncode)
	{
		delete m_pImageEncode;
		m_pImageEncode = nullptr;
	}

	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

void CRemultiplexer::SetAudioResampler()
{
	m_pAudioEncode->SetResampler();
}

void CRemultiplexer::SendAudioFrame(CAVFrame* frame)
{
	m_pAudioEncode->PushFrameToFIFO(frame);
}

void CRemultiplexer::SendAudioFrame(AVFrame* frame)
{
	m_pAudioEncode->PushFrameToFIFO(frame);
	if (frame)
		av_frame_free(&frame);
}

void CRemultiplexer::SendAudioFrame(uint8_t* pBuf, int BufSize)
{
	m_pAudioEncode->PushFrameToFIFO(pBuf, BufSize);
}

void CRemultiplexer::RemuxEvent(AVPacket* pkt, int nType, int64_t pts)
{
	int ret = 0;
	if (nType == AVMEDIA_TYPE_AUDIO)
	{
		if (pkt)
		{
			av_packet_rescale_ts(pkt, m_AudioTimeBase, m_pAudioStream->time_base);
			pkt->stream_index = m_pAudioStream->index;
		}

		m_AudioPacket.MaxSizePush(pkt, &m_bRun);
	}
	else if (nType == AVMEDIA_TYPE_VIDEO)
	{

	}
}

void CRemultiplexer::Work()
{
	AVPacket* pkt = nullptr;

	// 写文件头
	avformat_write_header(m_pFormatCtx, nullptr);
	printf("开始写入文件头.\r\n");

	while (m_bRun)
	{
		if (m_AudioPacket.Pop(pkt))
		{
			if (pkt)
			{
				av_write_frame(m_pFormatCtx, pkt);
				av_packet_free(&pkt);
			}
			else
			{
				m_bRun = false;
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	av_write_trailer(m_pFormatCtx);

	Cleanup();
	printf("文件写入成功.\n");
}
