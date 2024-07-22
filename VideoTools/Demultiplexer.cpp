#include "Demultiplexer.h"

CDemultiplexer::CDemultiplexer()
{

}

CDemultiplexer::~CDemultiplexer()
{

}

bool CDemultiplexer::Open(const std::string& strFile)
{
	if (0 > avformat_open_input(&m_pFormatCtx, strFile.c_str(), nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	m_videoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	m_audioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

	return true;
}

void CDemultiplexer::Start(IDemuxEvent* pEvt)
{
	m_demux_event = pEvt;

	m_bRun = true;
	m_demuxThread = std::thread(&CDemultiplexer::Work, this);
}

void CDemultiplexer::Close()
{
	m_bRun = false;

	if (m_demuxThread.joinable())
		m_demuxThread.join();

	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

void CDemultiplexer::Work()
{
	AVPacket* pkt = av_packet_alloc();
	int ret = 0;

	while (m_bRun)
	{
		ret = av_read_frame(m_pFormatCtx, pkt);
		if (ret == 0)
		{
			if (pkt->stream_index == m_videoIndex)
			{
				m_demux_event->DuxPacket(pkt, AVMEDIA_TYPE_VIDEO);
			}
			else if (pkt->stream_index == m_audioIndex)
			{
				m_demux_event->DuxPacket(pkt, AVMEDIA_TYPE_AUDIO);
			}
			else if (pkt->stream_index == m_subtitleIndex)
			{

			}
		}
		else
		{
			if (ret == AVERROR_EOF)
			{
				if (m_videoIndex >= 0)
				{
					m_demux_event->DuxPacket(nullptr, AVMEDIA_TYPE_VIDEO);
				}
				if (m_audioIndex >= 0)
				{
					m_demux_event->DuxPacket(nullptr, AVMEDIA_TYPE_AUDIO);
				}
				m_bRun = false;			
			}
		}
		av_packet_unref(pkt);			
	}

	if (m_demux_event)
		m_demux_event->DuxEnd();

	av_packet_free(&pkt);
	printf("[CDemultiplexer] -> Input file demultiplexing completed......\r\n");
}
