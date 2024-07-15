#pragma once
#include "Demultiplexer.h"
#include "Remultiplexer.h"
#include "AudioDecoder.h"

class CTranscode : public IDemuxEvent, public IDecoderEvent
{
public:
	CTranscode();
	~CTranscode();

	bool OpenInput(const std::string& strFile);

	void SetOutput(const std::string& strFile);

	void Run();

protected:
	virtual void DuxStart() override;
	virtual void DuxPacket(AVPacket* _data, int _type) override;
	virtual void DuxEnd() override;

	virtual void VideoEvent(AVFrame* _img, class CVideoDecoder* _decoder) override;
	virtual void AudioEvent(uint8_t* _buf, int _len, int64_t _pts, double _timebase, double _rate) override;
	virtual void AudioEvent(AVFrame* frame) override;

protected:
	void Work();

	void Close();

private:
	CDemultiplexer	m_demux;
	CRemultiplexer	m_remux;
	CAudioDecoder	m_AudioDecoder;

	// 音频解码结果缓冲队列
	SafeQueue<CAVFrame*> m_AudioData;
	SafeQueue<AVFrame*> m_AudioFrameData;

	bool m_bRun = false;
	std::thread m_thread;
};

