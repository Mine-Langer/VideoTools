#pragma once
#include "Common.h"

// �⸴����ػص��ӿ�
class IDemuxEvent
{
public:
	virtual void DuxStart() PURE;
	virtual void DuxPacket(AVPacket* _data, int _type) PURE;
	virtual void DuxEnd() PURE;
};

// ��Ƶ������ػص��ӿ�
class IDecoderEvent
{
public:
	virtual void VideoEvent(AVFrame* _img, class CVideoDecoder* _decoder) = 0;
	virtual void AudioEvent(uint8_t* _buf, int _len, int64_t _pts, double _timebase, double _rate) = 0;
	virtual void AudioEvent(AVFrame* frame) = 0;
};

// ��������Ƶ�ص��ӿ�
class IPlayerEvent
{
public:
	virtual void PlayImageEvent(AVFrame* image) = 0;
	
	virtual void PlayAudioEvent(AVFrame* _frame) = 0;
};

// ��Ƶ�⸴��ģ��
class CDemultiplexer
{
public:
	CDemultiplexer();
	~CDemultiplexer();

	bool Open(const std::string& strFile);

	void Start(IDemuxEvent* pEvt);

	void Close();

	AVFormatContext* GetFormatCtx() { return m_pFormatCtx; }
	int GetAudioIdx() const { return m_audioIndex; }
	int GetVideoIdx() const { return m_videoIndex; }

protected:
	void Work();
		
private:
	AVFormatContext* m_pFormatCtx = nullptr;

	int m_videoIndex = -1;
	int m_audioIndex = -1;
	int m_subtitleIndex = -1;

	bool m_bRun = false;
	bool m_videoEnable = false;
	bool m_audioEnable = false;

	IDemuxEvent* m_demux_event;

	std::thread m_demuxThread;
};

