#pragma once
#include "CVideoDecoder.h"

class CRecorder : public IDecoderEvent
{
public:
	CRecorder();
	~CRecorder();

	bool Run();

	// 初始化录制视频
	bool InitVideo();

	bool InitOutput(const char* szOutput);

private:
	void Start(); // 开启解复用
	void OnDemuxThread(); // 解复用线程

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
	int StreamFrameRate = 0; // 刷新频率

	CVideoDecoder m_videoDecoder;

	std::thread m_demuxThread;

	SafeQueue<AVFrame*> VideoFrameData;
};

