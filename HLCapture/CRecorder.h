#pragma once
#include "CVideoDecoder.h"

class CRecorder : public IDecoderEvent
{
public:
	CRecorder();
	~CRecorder();

	bool Run();

	// ��ʼ��¼����Ƶ
	bool InitVideo();

	bool InitOutput(const char* szOutput);

private:
	void Start(); // �����⸴��
	void OnDemuxThread(); // �⸴���߳�

	bool InitVideoOutput();
	bool InitAudioOutput();

private:
	void VideoEvent(AVFrame* vdata) override;

	void AudioEvent(STAudioBuffer* adata) override;

private:
	bool m_bRun = false;
	AVFormatContext* OutputFormatCtx = nullptr;
	AVFormatContext* VideoFormatCtx = nullptr;
	AVFormatContext* AudioFormatCtx = nullptr;

	AVOutputFormat* OutputFormat = nullptr;

	int VideoIndex = -1;
	int AudioIndex = -1;
	int StreamFrameRate = 0; // ˢ��Ƶ��

	CVideoDecoder m_videoDecoder;

	std::thread m_demuxThread;

	SafeQueue<AVFrame*> VideoFrameData;
};

