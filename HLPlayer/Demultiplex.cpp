#include "Demultiplex.h"

CDemultiplex::CDemultiplex()
{

}

CDemultiplex::~CDemultiplex()
{

}

bool CDemultiplex::Open(const char* szFile)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szFile, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	m_audioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	m_videoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	m_subtitleIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_SUBTITLE, -1, -1, nullptr, 0);
	
	return true;
}

void CDemultiplex::Start(IDemuxEvent* pEvent)
{
	m_pDemuxEvent = pEvent;

	m_avStatus = ePlay;
	m_demuxThread = std::thread(&CDemultiplex::OnDemuxThread, this);
	m_demuxThread.detach();
}

void CDemultiplex::Close()
{
	m_avStatus = eStop;
	if (m_demuxThread.joinable())
		m_demuxThread.join();

	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

AVStream* CDemultiplex::VideoStream()
{
	if (m_videoIndex < 0)
		return nullptr;

	return m_pFormatCtx->streams[m_videoIndex];
}

AVStream* CDemultiplex::AudioStream()
{
	if (m_audioIndex < 0)
		return nullptr;

	return m_pFormatCtx->streams[m_audioIndex];
}

AVStream* CDemultiplex::SubtitleStream()
{
	if (m_subtitleIndex < 0)
		return nullptr;

	return m_pFormatCtx->streams[m_subtitleIndex];
}

void CDemultiplex::OnDemuxThread()
{
	while (m_avStatus != eStop)
	{
		if (m_avStatus == ePause)
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			AVPacket pkt;
			if (0 > av_read_frame(m_pFormatCtx, &pkt))
			{
				m_pDemuxEvent->OnDemuxPacket(nullptr, AVMEDIA_TYPE_VIDEO);
				m_pDemuxEvent->OnDemuxPacket(nullptr, AVMEDIA_TYPE_AUDIO);
				m_pDemuxEvent->OnDemuxPacket(nullptr, AVMEDIA_TYPE_SUBTITLE);
				break;
			}

			if (pkt.stream_index == m_videoIndex)
			{
				m_pDemuxEvent->OnDemuxPacket(&pkt, AVMEDIA_TYPE_VIDEO);
			}
			else if (pkt.stream_index == m_audioIndex)
			{
				m_pDemuxEvent->OnDemuxPacket(&pkt, AVMEDIA_TYPE_AUDIO);
			}
			else if (pkt.stream_index == m_subtitleIndex)
			{
				m_pDemuxEvent->OnDemuxPacket(&pkt, AVMEDIA_TYPE_SUBTITLE);
			}
		}
	}
}
