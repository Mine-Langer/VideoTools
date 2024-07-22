
#include "Transcoder.h"

CTranscoder::CTranscoder(bool enableVideo, bool enableAudio):m_videoEnable(enableVideo), m_audioEnable(enableAudio)
{
}

CTranscoder::~CTranscoder()
{

}

bool CTranscoder::OpenInputFile(const std::string& strFile)
{
	if (!m_demux.Open(strFile))
		return false;

	if (!m_video.Open(m_demux))
		return false;

	if (!m_audio.Open(m_demux))
		return false;

	return true;
}

bool CTranscoder::OpenOutputFile(const std::string& strFile, bool ba, bool bv)
{
	if (!m_remux.Open(strFile, ba, bv, m_OutWidth, m_OutHeight))
		return false;

	return true;
}

void CTranscoder::SetOutputFormat(int w, int h)
{
	m_OutWidth = w;
	m_OutHeight = h;

	int sample_rate, nb_samples;
	AVChannelLayout ch_layout;
	enum AVSampleFormat sample_fmt;
	m_audio.GetSrcParameter(sample_rate, ch_layout, sample_fmt);

	m_audio.SetSwr(ch_layout, sample_fmt, sample_rate, nb_samples);

	m_video.InitSwsContext(m_OutWidth, m_OutHeight);
}

bool CTranscoder::Start(ITranscodeProgress* pEvt)
{
	m_pTransEvent = pEvt;

	m_demux.Start(this);

	if (m_videoEnable)
		m_video.Start(this);

	if (m_audioEnable)
		m_audio.Start(this);

	m_remux.Start(pEvt);

	m_bRun = true;
	m_thread = std::thread(&CTranscoder::Work, this);

	return true;
}

void CTranscoder::Close()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();
}

void CTranscoder::DuxPacket(AVPacket* _data, int _type)
{
	if (_type == AVMEDIA_TYPE_VIDEO)
	{
		m_video.SendPacket(_data);
	}
	else if (_type == AVMEDIA_TYPE_AUDIO)
	{
		m_audio.SendPacket(_data);
	}
}

void CTranscoder::VideoEvent(AVFrame* _img, class CVideoDecoder* _decoder)
{
	AVFrame* tframe = nullptr;
	if (_img)
		tframe = av_frame_clone(_img);
	m_videoFrameData.MaxSizePush(tframe, &m_bRun);
}

void CTranscoder::AudioEvent(AVFrame* frame)
{
	AVFrame* tframe = nullptr;
	if (frame)
		tframe = av_frame_clone(frame);
	m_AudioFrameData.MaxSizePush(tframe, &m_bRun);
}

void CTranscoder::Work()
{
	int ret = 0;
	AVFrame* frame_a = nullptr;
	AVFrame* frame_v = nullptr;
	int a, v;

	while (m_bRun)
	{
		if (m_audioEnable && m_AudioFrameData.Pop(frame_a))
		{
			a = 2;
			if (frame_a == nullptr)
				a = 1;
			else
			{
				m_remux.SendAudioFrame(frame_a);
				av_frame_free(&frame_a);
			}			
		}
		else
		{
			if (a != 1) a = 0;
		}

		if (m_videoEnable && m_videoFrameData.Pop(frame_v))
		{
			v = 2;
			if (frame_v == nullptr)
				v = 1;
			else
			{
				m_remux.SendVideoFrame(frame_v);
				av_frame_free(&frame_v);
			}
				
		}
		else
		{
			if (v != 1) v = 0;
		}

		// 数据为空 跳出循环
		if (a == 1 && v == 1)
			break;
		else if (a == 0 && v == 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	printf("Format conversion data submission completed...\r\n");
}
