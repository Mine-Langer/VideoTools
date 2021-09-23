#pragma once
#include "decoder.h"

class CAudioDecoder :public CDecoder
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	bool Open(AVStream* pStream);

	void Start(IDecoderEvent* evt);

	bool SendPacket(AVPacket* pkt) override;

	bool SetConfig();

	void Close() override;

public:
	int GetSampleRate();
	int GetChannels();
	int GetSamples();

private:
	void OnDecodeFunction() override;

private:
	AVCodecContext* AudioCodecCtx = nullptr;

	AVFrame* SrcFrame = nullptr;
	SwrContext* SwrCtx = nullptr;

	int64_t m_channel_layout = 0; // 声道布局
	AVSampleFormat m_sample_fmt = AV_SAMPLE_FMT_NONE; // 采样格式
	int m_sample_rate = 0; // 采样率
	int m_nb_samples = 0; // 采样数
};

