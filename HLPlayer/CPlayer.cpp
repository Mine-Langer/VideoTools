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
		m_audioDecoder;
	}

	return true;
}

void CPlayer::Start()
{
	m_bRun = true;
	m_ReadThread = std::thread(&CPlayer::OnReadFunction, this);
}

void CPlayer::OnReadFunction()
{
	while (m_bRun)
	{

	}
}
