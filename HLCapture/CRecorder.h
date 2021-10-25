#pragma once
#include "CVideoDecoder.h"
#include "CVideoEncoder.h"
#include "CAudioDecoder.h"
#include "CAudioEncoder.h"

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

	void Pause(); // 暂停

	void Resume(); // 继续

	void Stop(); // 终止

private:
	void Start(); // 开启解复用
	void OnDemuxVideoThread(); // 图像解复用线程
	void OnDemuxAudioThread(); // 录音解复用线程
	void OnSaveThread(); // 保存视频帧线程

	bool InitVideoOutput();
	bool InitAudioOutput();

private:
	void VideoEvent(AVFrame* vdata) override;
	void AudioEvent(STAudioBuffer* adata) override;

	void VideoEvent(AVPacket* vdata) override;
	void AudioEvent(AVPacket* adata) override;

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
	CAudioDecoder m_audioDecoder;
	CAudioEncoder m_audioEncoder;

	std::thread m_demuxVThread;
	std::thread m_demuxAThread;
	std::thread m_saveThread;

	SafeQueue<AVFrame*> VideoFrameData;
	SafeQueue<AVPacket*> VideoPacketData;
};

