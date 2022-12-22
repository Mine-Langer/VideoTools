#pragma once
#include "Demultiplexer.h"
#include "VideoDecoder.h"

class CAudioDecoder 
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	// ��ָ����Ƶ�ļ�
	bool Open(const char* szInput);

	bool Open(CDemultiplexer* pDemux);

	// ����Ƶ�豸
	bool OpenMicrophone(const char* szUrl);

	bool Start(IDecoderEvent* pEvt);

	void Stop();

	void Clear();

	// �ȴ��������
	bool WaitFinished();

	bool SendPacket(AVPacket* pkt);

	void GetSrcParameter(int& sample_rate, AVChannelLayout& ch_layout, enum AVSampleFormat& sample_fmt);
	int SetSwrContext(AVChannelLayout ch_layout, enum AVSampleFormat sample_fmt, int sample_rate);
	
	AVChannelLayout GetChannelLayout();
	double Timebase();

	// ת��
	AVFrame* ConvertFrame(AVFrame* frame);

protected:

	void OnDecodeFunction();

	void Release();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	SwrContext* m_pSwrCtx = nullptr;
	IDecoderEvent* m_pEvent = nullptr;

	double m_timebase = 0.0;

	AVChannelLayout m_swr_ch_layout;
	AVSampleFormat m_swr_sample_fmt = AV_SAMPLE_FMT_NONE;
	int m_swr_sample_rate = 0;

	bool m_bRun = false;
	enum AVState m_state = NotStarted;
	std::thread m_thread;


	SafeQueue<AVPacket*> m_srcAPktQueue;
};

