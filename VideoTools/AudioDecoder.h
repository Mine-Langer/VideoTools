#pragma once
#include "Demultiplexer.h"
#include "VideoDecoder.h"

class CAudioDecoder:public IDemuxEvent
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	// 打开指定音频文件
	bool Open(const char* szInput);

	bool Open(CDemultiplexer* pDemux);

	// 打开音频设备
	bool OpenMicrophone(const char* szUrl);

	bool Start(IDecoderEvent* pEvt);

	void Stop();

	bool SendPacket(AVPacket* pkt);

	void GetSrcParameter(int& sample_rate, AVChannelLayout& ch_layout, enum AVSampleFormat& sample_fmt);
	bool SetSwrContext(AVChannelLayout ch_layout, enum AVSampleFormat sample_fmt, int sample_rate);
	
	AVChannelLayout GetChannelLayout();

	// 转码
	AVFrame* ConvertFrame(AVFrame* frame);

protected:
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;

	void OnDecodeFunction();

	void Release();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	SwrContext* m_pSwrCtx = nullptr;
	IDecoderEvent* m_pEvent = nullptr;

	int64_t m_channelLayout = 0;
	AVSampleFormat m_sampleFormat = AV_SAMPLE_FMT_NONE;
	int m_sampleRate = 0;
	int m_nbSamples = 0;

	bool m_bRun = false;
	enum AVState m_state = NotStarted;
	std::thread m_thread;


	SafeQueue<AVPacket*> m_srcAPktQueue;
};

