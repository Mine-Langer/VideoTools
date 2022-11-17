#include "Demultiplexer.h"

CDemultiplexer::CDemultiplexer()
{
	avformat_network_init();
}

CDemultiplexer::~CDemultiplexer()
{
	avformat_network_deinit();
}

bool CDemultiplexer::Open(const char* szInput)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szInput, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	for (int i = 0; i < m_pFormatCtx->nb_streams; i++)
	{
		if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoIndex = i;
		}
		else if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_audioIndex = i;
		}
	}

	return true;
}

bool CDemultiplexer::Open(const char* szInput, const AVInputFormat* ifmt, AVDictionary** pDict)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szInput, ifmt, pDict))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	for (int i = 0; i < m_pFormatCtx->nb_streams; i++)
	{
		if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoIndex = i;
		}
		else if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_audioIndex = i;
		}
	}

	return true;
}

void CDemultiplexer::Start(IDemuxEvent* pEvt)
{
	m_pEvent = pEvt;

	m_bRun = true;
	m_thread = std::thread(&CDemultiplexer::OnDemuxFunction, this);
}

void CDemultiplexer::Stop()
{
	m_bRun = false;

	if (m_thread.joinable())
		m_thread.join();

	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

int CDemultiplexer::AudioStreamIndex()
{
	return m_audioIndex;
}

int CDemultiplexer::VideoStreamIndex()
{
	return m_videoIndex;
}

AVFormatContext* CDemultiplexer::FormatContext()
{
	return m_pFormatCtx;
}

void CDemultiplexer::OnDemuxFunction()
{
	AVPacket* pkt = av_packet_alloc();

	while (m_bRun)
	{
		if (0 > av_read_frame(m_pFormatCtx, pkt))
		{
			m_pEvent->DemuxPacket(nullptr, AVMEDIA_TYPE_AUDIO);
			m_pEvent->DemuxPacket(nullptr, AVMEDIA_TYPE_VIDEO);
			m_bRun = false;
		}
		else
		{
			if (pkt->stream_index == m_audioIndex)
			{
				m_pEvent->DemuxPacket(pkt, AVMEDIA_TYPE_AUDIO);
			}
			else if (pkt->stream_index == m_videoIndex)
			{
				m_pEvent->DemuxPacket(pkt, AVMEDIA_TYPE_VIDEO);
			}
		}
	}
	av_packet_free(&pkt);
}
