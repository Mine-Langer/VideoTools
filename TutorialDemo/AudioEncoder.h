#pragma once
#include "Remultiplexer.h"

class CAudioEncoder
{
public:
	CAudioEncoder();
	~CAudioEncoder();

	static CAudioEncoder* CreateObject();

	void Close();

	AVCodecContext* Init(AVCodecID CodecId, AVSampleFormat SampleFmt=AV_SAMPLE_FMT_FLTP, int SampleRate=44100, int BitRate=128000);

	void Start(IRemuxEvent* pEvt);

	// ≈‰÷√“Ù∆µ÷ÿ≤…—˘
	bool SetResampler();

	void PushFrameToFIFO(uint8_t* pBuf, int BufSize);
	void PushFrameToFIFO(CAVFrame* frame);
	void PushFrameToFIFO(AVFrame* frame);

protected:
	void Work();

	AVFrame* AllocOutputFrame(int nbSize);

	void Cleanup();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	AVAudioFifo*	m_pAudioFifo = nullptr;
	SwrContext*		m_pSwrCtx = nullptr;

	IRemuxEvent* m_pRemuxEvt = nullptr;

	int m_nbSamples = 0;
	int64_t m_frameIndex = 0;

	bool m_bFinished = false;

	bool m_bRun = false;
	std::thread m_thread;
	std::mutex m_audioMutex;
};

