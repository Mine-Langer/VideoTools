#include "player.h"

CPlayer::CPlayer()
{

}

CPlayer::~CPlayer()
{

}

bool CPlayer::Open(const char* szInput)
{
	if (!m_demux.Open(szInput))
		return false;

	if (m_videoDecoder.Open(&m_demux))
	{
		//m_videoDecoder.GetSrcParameter();
	}

	if (m_audioDecoder.Open(&m_demux))
	{
		int sample_rate; 
		AVChannelLayout ch_layout; 
		AVSampleFormat sample_fmt;
		m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
		m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);
	}

	return true;
}

void CPlayer::SetView(HWND hWnd, int w, int h)
{
	m_videoDecoder.SetSwsConfig(w, h);
}

void CPlayer::Start()
{
	m_demux.Start(this);
	m_videoDecoder.Start(this);
	m_audioDecoder.Start(this);

	m_bRun = true;
	m_tPlay = std::thread(&CPlayer::OnPlayProc, this);
}

void CPlayer::Stop()
{

}

void CPlayer::Release()
{

}

void CPlayer::OnPlayProc()
{
	AVFrame* pFrame = nullptr;

	while (m_bRun)
	{
		m_audioFrameQueue.Pop(pFrame);
		if (pFrame)
			m_dxAudio.PushPCM(pFrame->extended_data[0], pFrame->nb_samples);
	}
}

bool CPlayer::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_VIDEO)
	{
		m_videoDecoder.SendPacket(pkt);
	}
	else if (type == AVMEDIA_TYPE_AUDIO)
	{
		m_audioDecoder.SendPacket(pkt);
	}
	return true;
}

bool CPlayer::VideoEvent(AVFrame* frame)
{
	m_videoFrameQueue.MaxSizePush(frame, &m_bRun);

	return true;
}

bool CPlayer::AudioEvent(AVFrame* frame)
{
	m_audioFrameQueue.MaxSizePush(frame, &m_bRun);

	return true;
}