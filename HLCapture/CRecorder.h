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

private:
	void Start();

	void OnDemuxThread();

private:
	void VideoEvent(AVFrame* vdata) override;

	void AudioEvent(STAudioBuffer* adata) override;

private:
	bool m_bRun = false;
	AVFormatContext* VideoFormatCtx = nullptr;


	int VideoIndex = -1;
	int AudioIndex = -1;

	CVideoDecoder m_videoDecoder;

	std::thread m_demuxThread;

	SafeQueue<AVFrame*> VideoFrameData;
};

