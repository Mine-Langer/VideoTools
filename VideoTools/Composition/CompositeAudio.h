#pragma once
#include "../Demultiplexer.h"

class CompositeAudio :public IDemuxEvent
{
public:
	CompositeAudio();
	~CompositeAudio();

	bool Open(const char* szInput);

	void GetSrcParameter(int& sample_rate, int& nb_sample, int64_t& ch_layout,enum AVSampleFormat& sample_fmt);
	bool SetSwrContext(int64_t ch_layout, enum AVSampleFormat sample_fmt, int sample_rate);

	void Start(IAudioEvent* pEvt);

	void Release();

private:
	void OnDecodeFunction();

	bool DemuxPacket(AVPacket* pkt, int type) override;

private:
	CDemultiplexer m_demux;

	AVCodecContext* m_codecCtx = nullptr;
	SwrContext* m_swrCtx = nullptr;

	SafeQueue<AVPacket*> m_audioQueue;
	
	IAudioEvent* m_pEvent = nullptr;

	int64_t m_ch_layout = 0;
	AVSampleFormat m_sample_fmt = AV_SAMPLE_FMT_NONE;
	int m_sample_rate = 0;
	int m_nb_samples = 0;

	bool m_bRun = false;
	std::thread m_thread;
};

