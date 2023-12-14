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
	if (0 != avformat_open_input(&m_pFormatCtx, szInput, NULL, NULL))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	m_videoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	m_audioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	
	return true;
}

bool CDemultiplexer::Open(const char* szInput, const AVInputFormat* ifmt, AVDictionary** pDict)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szInput, ifmt, pDict))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	m_videoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	m_audioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	
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

void CDemultiplexer::Release()
{
	if (m_pFormatCtx) {
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

void CDemultiplexer::WaitFinished()
{
	if (m_thread.joinable())
		m_thread.join();

	Release();
}

void CDemultiplexer::SetPosition(int64_t dwTime)
{
	m_target_pts = dwTime;
	m_seek = true;
}

void CDemultiplexer::SetUniqueStream(bool bUnique, int uniqueStream)
{
	m_bUniqueStream = bUnique;
	m_uniqueIndex = uniqueStream;
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
	int aidx = 0, vidx = 0;
	int err = 0;
	while (m_bRun)
	{
		if (m_seek)
		{
			avformat_seek_file(m_pFormatCtx, -1, INT64_MIN, m_target_pts, INT64_MAX, 0);
			m_pEvent->CleanPacket();
			m_seek = false;
		}

		AVPacket* pkt = av_packet_alloc();
		if (0 > (err = av_read_frame(m_pFormatCtx, pkt)))
		{
			av_packet_free(&pkt);
			if (err == AVERROR_EOF)
			{
				if (m_audioIndex >= 0)
					m_pEvent->DemuxPacket(nullptr, AVMEDIA_TYPE_AUDIO);
				if (m_videoIndex >= 0)
					m_pEvent->DemuxPacket(nullptr, AVMEDIA_TYPE_VIDEO);
				m_bRun = false;
				printf("input file demux over. \n");
			}
		}
		else
		{
			if (pkt->stream_index == m_audioIndex)
			{
				if (m_bUniqueStream && m_uniqueIndex == AVMEDIA_TYPE_AUDIO)
					continue;
				//printf("	demux a audio packet. %d\n", aidx++);
				m_pEvent->DemuxPacket(pkt, AVMEDIA_TYPE_AUDIO);
			}
			else if (pkt->stream_index == m_videoIndex)
			{
				if (m_bUniqueStream && m_uniqueIndex == AVMEDIA_TYPE_VIDEO)
					continue;
				//printf("	demux a video packet. %d\n", vidx++);
				m_pEvent->DemuxPacket(pkt, AVMEDIA_TYPE_VIDEO);
			}
			printf("pkt idx[%d] pkt times:%.2f  duration:%.2f\n", pkt->stream_index,
				av_q2d(m_pFormatCtx->streams[pkt->stream_index]->time_base)*pkt->pts,
				av_q2d(m_pFormatCtx->streams[pkt->stream_index]->time_base) * pkt->duration);
		}
	}

	Release();
}
