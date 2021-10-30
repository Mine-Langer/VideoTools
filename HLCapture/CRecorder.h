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

	// ��ʼ��¼����Ƶ
	bool InitVideo();

	bool InitAudio();

	bool InitOutput(const char* szOutput);

	void Pause(); // ��ͣ

	void Resume(); // ����

	void Stop(); // ��ֹ

private:
	void Start(); // �����⸴��
	void OnDemuxVideoThread(); // ͼ��⸴���߳�
	void OnDemuxAudioThread(); // ¼���⸴���߳�
	void OnSaveThread(); // ������Ƶ֡�߳�

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
	int StreamFrameRate = 0; // ˢ��Ƶ��

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

