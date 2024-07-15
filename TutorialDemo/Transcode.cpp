#include "Transcode.h"

CTranscode::CTranscode()
{

}

CTranscode::~CTranscode()
{
	Close();
}

bool CTranscode::OpenInput(const std::string& strFile)
{
	m_demux.Open(strFile);
	
	m_AudioDecoder.Open(m_demux);

	int sample_rate;
	AVChannelLayout ch_layout; 
	enum AVSampleFormat sample_fmt;
	
	m_AudioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
	m_AudioDecoder.SetSwr(ch_layout, sample_fmt, sample_rate);

	return true;
}

void CTranscode::SetOutput(const std::string& strFile)
{
	m_remux.Open(strFile);

}

void CTranscode::Run()
{
	m_demux.Start(this);

	m_AudioDecoder.Start(this);

	m_remux.Start();

	m_bRun = true;
	m_thread = std::thread(&CTranscode::Work, this);
}

void CTranscode::DuxStart()
{

}

void CTranscode::DuxPacket(AVPacket* _data, int _type)
{
	switch (_type)
	{
	case AVMEDIA_TYPE_AUDIO:
		m_AudioDecoder.SendPacket(_data);
		break;
	case AVMEDIA_TYPE_VIDEO:
		break;
	}
}

void CTranscode::DuxEnd()
{

}

void CTranscode::VideoEvent(AVFrame* _img, class CVideoDecoder* _decoder)
{

}

void CTranscode::AudioEvent(uint8_t* _buf, int _len, int64_t _pts, double _timebase, double _rate)
{
	CAVFrame* frame = new CAVFrame();

	frame->CopyPCM(_buf, _len);
	frame->data_len[0] = _len;
	frame->pts = _pts;
	frame->dpts = _pts * _timebase;
	frame->duration = 1.0 / _rate;

	m_AudioData.MaxSizePush(frame, &m_bRun);
}

void CTranscode::AudioEvent(AVFrame* frame)
{
	// 防止AudioFIFO溢出  Audio队列不能太多
	m_AudioFrameData.MaxSizePush(frame, &m_bRun, 5);
}

void CTranscode::Work()
{
	int ret = 0;
	AVFrame* frame = nullptr;

	while (m_bRun)
	{
		if (m_AudioFrameData.Pop(frame))
		{
			if (frame == nullptr)
			{
				m_remux.SendAudioFrame(frame);
				m_bRun = false;
			}
			else
			{
				m_remux.SendAudioFrame(frame);		
				// av_frame_free(&frame);
			}		
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	printf("输入文件音频转码完成。。。。。\r\n");
}

void CTranscode::Close()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();

	m_AudioDecoder.Close();

	m_demux.Close();

	m_remux.Close();
}
