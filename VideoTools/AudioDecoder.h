#pragma once
#include "Demultiplexer.h"

class CAudioDecoder:public IDemuxEvent
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	bool Open(const char* szInput);

	bool Start(IAudioEvent* pEvt);

	void GetSrcParameter(int& sample_rate, int& nb_sample, int64_t& ch_layout, enum AVSampleFormat& sample_fmt);
	bool SetSwrContext(int64_t ch_layout, enum AVSampleFormat sample_fmt, int sample_rate);

	void SetSaveEnable(bool isSave);

	void Release();
protected:
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;
	void OnDecodeFunction();
	// зЊТы
	AVFrame* ConvertFrame(AVFrame* frame);

private:
	CDemultiplexer m_demux;
	AVCodecContext* m_pCodecCtx = nullptr;
	SwrContext* m_pSwrCtx = nullptr;
	IAudioEvent* m_pEvent = nullptr;

	int64_t m_channelLayout = 0;
	AVSampleFormat m_sampleFormat = AV_SAMPLE_FMT_NONE;
	int m_sampleRate = 0;
	int m_nbSamples = 0;

	enum AVState m_state = NotStarted;
	std::thread m_thread;

	bool m_bIsSave = false;
	SafeQueue<AVPacket*> m_srcAPktQueue;
};

