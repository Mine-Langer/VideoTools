#pragma once
#include "Demultiplexer.h"
#include "Remultiplexer.h"
#include "VideoDecoder.h"
#include "AudioDecoder.h"

class CTranscoder : public IDemuxEvent, public IDecoderEvent
{
public:
	CTranscoder(bool enableVideo = true, bool enableAudio = true);
	~CTranscoder();

	// 打开输入文件
	bool OpenInputFile(const std::string& strFile);

	// 打开输出文件
	bool OpenOutputFile(const std::string& strFile, bool ba, bool bv);

	// 设置输出格式
	void SetOutputFormat(int w=-1, int h=-1);
	
	// 开始执行转换
	bool Start(ITranscodeProgress* pEvt);

	void Close();

protected:
	virtual void DuxStart() {}
	virtual void DuxPacket(AVPacket* _data, int _type) override;
	virtual void DuxEnd() {}

	virtual void VideoEvent(AVFrame* _img, class CVideoDecoder* _decoder) override;
	virtual void AudioEvent(uint8_t* _buf, int _len, int64_t _pts, double _timebase, double _rate) {}
	virtual void AudioEvent(AVFrame* frame) override;

protected:
	void Work();


private:
	ITranscodeProgress* m_pTransEvent = nullptr;
	CDemultiplexer	m_demux;
	CRemultiplexer	m_remux;
	CVideoDecoder	m_video;
	CAudioDecoder	m_audio;

	SafeQueue<AVFrame*> m_AudioFrameData; // 解码后的PCM Audio Data 
	SafeQueue<AVFrame*> m_videoFrameData; // 解码后的YUV Image Data

	int m_OutWidth;
	int m_OutHeight;

	bool m_audioEnable = false;
	bool m_videoEnable = false;
	bool m_bRun = false;
	std::thread m_thread;
};

