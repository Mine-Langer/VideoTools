#pragma once
#include "Demultiplexer.h"

class CAudioDecoder
{
public:
	CAudioDecoder();
	~CAudioDecoder();

	bool Open(const std::string& strFile);
	bool Open(CDemultiplexer& demux);

	void Start(IDecoderEvent* pEvt);
		
	void Close();

	// ��ȡԭʼ��Ƶ��(PCM) �Ĳ����ʡ���֡������������
	void GetSrcParameter(int& sample_rate, AVChannelLayout& ch_layout, enum AVSampleFormat& sample_fmt);


	// �����ز������� 
	// @param	ch_layout		��������
	// @param	sample_fmt		�ز�����ʽ
	// @param	sample_rate		�ز���������
	bool SetSwr(AVChannelLayout ch_layout, enum AVSampleFormat sample_fmt, int sample_rate, int& nb_samples);

	bool SendPacket(AVPacket* pkt);

	double GetTimebase() const;

protected:
	void Work();

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	SwrContext* m_pSwrCtx = nullptr;

	//AVFrame* m_srcFrame = nullptr;
	//AVFrame* m_dstFrame = nullptr;

	IDecoderEvent* m_DecoderEvt;

	AVChannelLayout		m_swr_ch_layout;
	enum AVSampleFormat	m_swr_sample_fmt;
	int					m_swr_sample_rate;
	int					m_swr_nb_samples;

	double m_timebase;
	double m_duration;
	double m_rate;

	SafeQueue<AVPacket*> m_PktQueue;

	bool m_bRun = false;
	std::thread m_DecodeThread;
};

