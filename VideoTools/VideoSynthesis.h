#pragma once
#include "Demultiplexer.h"
#include "Remultiplexer.h"
#include "VideoDecoder.h"
#include "AudioDecoder.h"
#include "player.h"
#include "VideoFilter.h"

/**
 * ��Ƶ�ϳ�
 */
class CVideoSynthesis: public IDemuxEvent, public IDecoderEvent
{
public:
	CVideoSynthesis();
	~CVideoSynthesis();

	void AddImage(const std::string& szFile);

	void AddAudio(const std::string& szFile);

	// ����Ⱦͼ��Ķ���ӿ�
	void BindShowWindow(HWND hWnd, int width, int height);

	void Start();

	void Close();

protected:
	void Work();
	void Work_Video();
	void Work_Video2();


protected:
	// �⸴�� ���ݽӿ�
	virtual void DuxStart() {}
	virtual void DuxPacket(AVPacket* _data, int _type) override;
	virtual void DuxEnd() {}

	// �������ݽӿ�
	virtual void AudioEvent(AVFrame* frame) override;
	virtual void VideoEvent(AVFrame* _img, class CVideoDecoder* _decoder) override;
	virtual void AudioEvent(uint8_t* _buf, int _len, int64_t _pts, double _timebase, double _rate) {}

private:
	CDemultiplexer	m_demux;
	CAudioDecoder	m_audioDecoder;
	CDemultiplexer	m_demux_image;
	CVideoDecoder	m_videoDecoder;

	CVideoFilter	m_videoFilter;

	CPlayer			m_player;



	std::vector<std::string>	m_vecImageList;
	std::vector<std::string>	m_vecVideoList;
	std::vector<std::string>	m_vecAudioList;

	SafeQueue<AVPacket*> m_VideoDataQueue;

	bool m_bRun = false;
	std::thread m_thread;
	std::thread m_vthread;
	std::thread m_vthread2;
};

