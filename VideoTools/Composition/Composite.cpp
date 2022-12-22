#include "Composite.h"

Composite::Composite()
{

}

Composite::~Composite()
{

}

void Composite::OpenImage(std::vector<ItemElem> vecImage)
{
	for (int i = 0; i < vecImage.size(); i++)
	{
		std::string szFilename = vecImage[i].filename.toStdString();

		if (m_demuxImage.Open(szFilename.c_str()))
		{
			m_videoDecoder.Open(&m_demuxImage);
			m_videoDecoder.SetSwsConfig(nullptr, m_outputWidth, m_outputHeight);

			m_demuxImage.Start(this);

			m_videoDecoder.Start(this);
			
			m_videoDecoder.WaitFinished();
			
			m_demuxImage.WaitFinished();
		}
	}
}


void Composite::OpenAudio(std::vector<ItemElem> vecAudio)
{
	for (int i = 0; i < vecAudio.size(); i++)
	{
		std::string szFilename = vecAudio[i].filename.toStdString();
		if (m_demuxMusic.Open(szFilename.c_str()))
		{
			m_demuxMusic.SetUniqueStream(true, AVMEDIA_TYPE_VIDEO);
			m_audioDecoder.Open(&m_demuxMusic);
			m_audioDecoder.SetSwrContext(m_out_ch_layout, m_out_sample_fmt, m_out_sample_rate);

			m_demuxMusic.Start(this);

			m_audioDecoder.Start(this);

			m_demuxMusic.WaitFinished();

			m_audioDecoder.WaitFinished();
		}
	}
}


void Composite::Close()
{	
	m_videoDecoder.Stop();
}

void Composite::Play(std::vector<ItemElem>& vecImage, std::vector<ItemElem>& vecMusic)
{
	m_nType = 1; // ²¥·Å

	m_outputWidth = m_videoWidth;
	m_outputHeight = m_videoHeight;

	m_player.SetView(m_hWndView, m_videoWidth, m_videoHeight);
	
	m_tv_demux = std::thread(&Composite::OpenImage, this, vecImage);

	m_ta_demux = std::thread(&Composite::OpenAudio, this, vecMusic);

	m_player.Start();
	/*SDL_Rect rect;
	for (int i = 0; i < vecImage.size(); i++)
	{
		std::string strFile = vecMusic[i].filename.toStdString();
		m_demuxImage.Open(strFile.c_str());
		if (m_videoDecoder.Open(&m_demuxImage))
		{
			m_videoDecoder.SetSwsConfig(&rect, m_videoWidth, m_videoHeight);

			m_demuxImage.Start(this);

			m_videoDecoder.Start(this);
		}
	}

	for (int i = 0; i < vecMusic.size(); i++)
	{
		std::string strFile = vecMusic[i].filename.toStdString();
		m_demuxMusic.Open(strFile.c_str());
		
		if (m_audioDecoder.Open(&m_demuxMusic))
		{
			int sample_rate;
			AVChannelLayout ch_layout;
			AVSampleFormat sample_fmt;
			m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);

			int nbSamples = m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);

			m_player.SetAudioSpec(sample_rate, ch_layout, nbSamples);
		}

		m_demuxMusic.Start(this);

		m_audioDecoder.Start(this);
		
		m_player.PlayAudio();
	}*/

}

bool Composite::InitWnd(HWND pWnd, int width, int height)
{
	m_hWndView = pWnd;
	m_videoWidth = width;
	m_videoHeight = height;

	return true;
}

bool Composite::SaveFile(const char* szOutput, std::vector<ItemElem>& vecImage, std::vector<ItemElem>& vecMusic)
{
	m_nType = 2;
	av_channel_layout_default(&m_out_ch_layout, 2);
	m_out_sample_rate = 44100;
	m_out_sample_fmt = AV_SAMPLE_FMT_FLTP;

 	m_tv_demux = std::thread(&Composite::OpenImage, this, vecImage);

	m_ta_demux = std::thread(&Composite::OpenAudio, this, vecMusic);

	if (!m_remux.SetOutput(szOutput, m_outputWidth, m_outputHeight, m_out_ch_layout, m_out_sample_fmt, m_out_sample_rate))
		return false;

	m_remux.Start(this);

	return true;
}

bool Composite::VideoEvent(AVFrame* frame)
{
	AVFrame* vdata = nullptr;
	if (frame != nullptr)
		vdata = av_frame_clone(frame);

	if (m_nType == 1)
		m_player.SendVideoFrame(vdata);
	else
		m_remux.SendFrame(vdata, AVMEDIA_TYPE_VIDEO);

	return true;
}

bool Composite::AudioEvent(AVFrame* frame)
{
	bool bRun = (m_state != Stopped);
	
	if (m_nType == 1)
		m_player.SendAudioFrame(frame);
	else
		m_remux.SendFrame(frame, AVMEDIA_TYPE_AUDIO);

	return true;
}

bool Composite::DemuxPacket(AVPacket* pkt, int type)
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

void Composite::CleanPacket()
{

}


void Composite::RemuxEvent(int nType)
{

}

void Composite::OnPlayFunction()
{

}
