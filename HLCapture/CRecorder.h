#pragma once
#include "CVideoDecoder.h"
#include "CVideoEncoder.h"

class CRecorder : public IDecoderEvent, public IEncoderEvent
{
public:
	CRecorder();
	~CRecorder();

	bool Run(const char* szFile);

	// 初始化录制视频
	bool InitVideo();

	bool InitAudio();

	bool InitOutput(const char* szOutput);

private:
	void Start(); // 开启解复用
	void OnDemuxThread(); // 解复用线程
	void OnSaveThread(); // 保存视频帧线程

	bool InitVideoOutput();
	bool InitAudioOutput();

private:
	void VideoEvent(AVFrame* vdata) override;
	void AudioEvent(STAudioBuffer* adata) override;

	void VideoEvent(AVPacket* vdata) override;

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
	CVideoEncoder m_videoEncoder;

	std::thread m_demuxThread;
	std::thread m_saveThread;

	SafeQueue<AVFrame*> VideoFrameData;
	SafeQueue<AVPacket*> VideoPacketData;
};

