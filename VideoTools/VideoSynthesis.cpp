#include "VideoSynthesis.h"

CVideoSynthesis::CVideoSynthesis()
{

}

CVideoSynthesis::~CVideoSynthesis()
{

}

void CVideoSynthesis::AddImage(const std::string& szFile)
{
	m_vecImageList.push_back(szFile);
}

void CVideoSynthesis::AddAudio(const std::string& szFile)
{
	m_vecAudioList.push_back(szFile);
}

void CVideoSynthesis::BindShowWindow(HWND hWnd, int width, int height)
{
	if (hWnd)
	{
		m_player.SetView(hWnd, width, height);
	}
}


void CVideoSynthesis::Start()
{
//	m_thread = std::thread(&CVideoSynthesis::Work, this);
	m_vthread = std::thread(&CVideoSynthesis::Work_Video, this);
}

void CVideoSynthesis::Work()
{
	for (int i = 0; i < m_vecAudioList.size(); i++)
	{
		m_demux.Open(m_vecAudioList[i]);

		m_audioDecoder.Open(m_demux);

		int sample_rate, nb_samples;
		AVChannelLayout ch_layout;
		enum AVSampleFormat sample_fmt;
		m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);

		m_audioDecoder.SetSwr(ch_layout, AV_SAMPLE_FMT_S16, sample_rate, nb_samples);

		m_player.SetAudioSpec(sample_rate, ch_layout, nb_samples);

		// ---------
		m_demux.Start(this);

		m_audioDecoder.Start(this);

		m_player.StartPlay();
	}	
}

void CVideoSynthesis::Work_Video()
{
	for (int i = 0; i < m_vecImageList.size(); i++)
	{
		m_demux_image.Open(m_vecImageList[i]);

		m_videoDecoder.Open(m_demux_image);
		m_videoDecoder.InitSwsContext();

		m_demux_image.Start(this);
		m_videoDecoder.Start(this);

		m_player.StartRender();
	}
}

void CVideoSynthesis::DuxPacket(AVPacket* _data, int _type)
{
	if (_type == AVMEDIA_TYPE_AUDIO)
	{
		m_audioDecoder.SendPacket(_data);
	}
	else if (_type == AVMEDIA_TYPE_VIDEO)
	{
		m_videoDecoder.SendPacket(_data);
	}
}

void CVideoSynthesis::AudioEvent(AVFrame* frame)
{
	AVFrame* tframe = nullptr;
	if (frame)
	{
		int realtime = frame->pts * m_audioDecoder.GetTimebase();
		printf("Synthesis -> audio decoder -> pts:%lld  realtime:%d  .\r\n", frame->pts, realtime);
		m_player.SendAudioFrame(frame);
	}
}

void CVideoSynthesis::VideoEvent(AVFrame* _img, class CVideoDecoder* _decoder)
{
	if (_img)
	{
		_img->sample_aspect_ratio;
		m_player.SendVideoFrame(_img);
	}
}
