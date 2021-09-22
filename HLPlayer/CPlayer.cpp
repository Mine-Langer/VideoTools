#include "CPlayer.h"

CPlayer::CPlayer()
{

}

CPlayer::~CPlayer()
{

}

bool CPlayer::Open(const char* szFile)
{
	if (0 != avformat_open_input(&FormatCtx, szFile, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(FormatCtx, nullptr))
		return false;

	VideoIndex = av_find_best_stream(FormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	AudioIndex = av_find_best_stream(FormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

	if (VideoIndex != -1) {
		m_videoDecoder.Open(FormatCtx->streams[VideoIndex], FormatCtx->streams[VideoIndex]->codecpar->codec_id);
	}

	if (AudioIndex != -1) {
		m_audioDecoder.Open(FormatCtx->streams[AudioIndex], FormatCtx->streams[AudioIndex]->codecpar->codec_id);
	}

	SrcPacket = av_packet_alloc();

	return true;
}

void CPlayer::Start()
{
	if (VideoIndex != -1)
		m_videoDecoder.Start();

	if (AudioIndex != -1)
		m_audioDecoder.Start();

	m_bRun = true;
	m_ReadThread = std::thread(&CPlayer::OnReadFunction, this);
}

void CPlayer::OnReadFunction()
{
	int error = 0;
	while (m_bRun)
	{
		if (0 > av_read_frame(FormatCtx, SrcPacket))
			break;
		
		if (SrcPacket->stream_index == VideoIndex)
		{
			m_videoDecoder.SendPacket(SrcPacket);
		}

		else if (SrcPacket->stream_index == AudioIndex)
		{
			m_audioDecoder.SendPacket(SrcPacket);
		}

		av_packet_unref(SrcPacket);
	}
}
