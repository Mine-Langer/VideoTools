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

bool CRemultiplexer::Open(const std::string& strFile, bool aEnable, bool vEnable, int width, int height)
{
	m_VideoEnable = vEnable;
	m_AudioEnable = aEnable;

	if (0 > avformat_alloc_output_context2(&m_pFormatCtx, nullptr, nullptr, strFile.c_str()))
		return false;

	const AVOutputFormat* pOutFormat = m_pFormatCtx->oformat;

	if (m_VideoEnable && pOutFormat->video_codec)
	{
		AVCodecContext* pCodecCtx = m_pImageEncode->Init(pOutFormat->video_codec, width, height);
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


void CRemultiplexer::Start(ITranscodeProgress* pEvt)
{
	m_pTransEvent = pEvt;

	m_pAudioEncode->Start(this);

	m_pImageEncode->Start(this);

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
}

void CRemultiplexer::SendAudioFrame(uint8_t* pBuf, int BufSize)
{
	m_pAudioEncode->PushFrameToFIFO(pBuf, BufSize);
}

void CRemultiplexer::SendVideoFrame(AVFrame* frame)
{
	m_pImageEncode->SendFrame(frame);
}

void CRemultiplexer::RemuxEvent(AVPacket* pkt, int nType, int64_t pts)
{
	int ret = 0;
	if (nType == AVMEDIA_TYPE_AUDIO)
	{
		if (pkt)
		{
		//	av_packet_rescale_ts(pkt, m_AudioTimeBase, m_pAudioStream->time_base);
			pkt->stream_index = m_pAudioStream->index;
		}

		m_AudioPacket.MaxSizePush(pkt, &m_bRun);
	}
	else if (nType == AVMEDIA_TYPE_VIDEO)
	{
		AVPacket* tpkt = nullptr;
		if (pkt)
		{
			tpkt = av_packet_clone(pkt);
		//	av_packet_rescale_ts(tpkt, m_VideoTimeBase, m_pVideoStream->time_base);
			tpkt->stream_index = m_pVideoStream->index;
		}
		m_videoPacket.MaxSizePush(tpkt, &m_bRun);
	}
}

void CRemultiplexer::Work()
{
	AVPacket* pkt_a = nullptr;
	AVPacket* pkt_v = nullptr;
	int a = 0, v = 0;

	// 写文件头
	avformat_write_header(m_pFormatCtx, nullptr);
	printf("start write file header.....\r\n");

	while (m_bRun)
	{
		if (m_AudioPacket.Pop(pkt_a))
		{
			if (pkt_a)
			{
				int real_time = pkt_a->pts * m_pAudioEncode->GetTimebase();
				//printf("remux audio -> pts:%lld  q2d:%f  realtime:%d ..\r\n", pkt_a->pts, q2d, real_time);

				av_packet_rescale_ts(pkt_a, m_AudioTimeBase, m_pAudioStream->time_base);
				av_interleaved_write_frame(m_pFormatCtx, pkt_a);

				m_pTransEvent->ProgressValue(real_time);

				av_packet_free(&pkt_a);

				a = 2;
			}
			else
			{
				a = 1;
			}
		}
		else
		{
			if (a != 1) a = 0;
		}

		if (m_videoPacket.Pop(pkt_v))
		{
			if (pkt_v)
			{
				av_packet_rescale_ts(pkt_v, m_VideoTimeBase, m_pVideoStream->time_base);
				av_interleaved_write_frame(m_pFormatCtx, pkt_v);
				av_packet_free(&pkt_v);
			}
			else
			{
				v = 1;
			}
		}
		else
		{
			if (v != 1) v = 0;
		}

		// 数据为空 跳出循环
		if (a == 1 && v == 1)
			m_bRun = false;
		else if (a == 0 && v == 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	av_write_trailer(m_pFormatCtx);

	Cleanup();
	printf("File Write overed.......\n");
}
