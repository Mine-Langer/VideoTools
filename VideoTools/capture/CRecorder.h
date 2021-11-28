#pragma once
#include "CapAudio.h"
#include "../VideoDecoder.h"


class CRecorder :public IVideoEvent, public IAudioEvent
{
public:
	CRecorder();
	~CRecorder();

	void Init(int posX, int posY, int sWidth, int sHeight);

	bool Run(const char* szFile);

	void Pause(); // 暂停

	void Resume(); // 继续

	void Stop(); // 终止

protected:
	virtual bool VideoEvent(AVFrame* frame) override;
	virtual bool AudioEvent(AVFrame* frame) override;

private:
	bool InitOutput(const char* szOutput);

	bool Start(); // 开启解复用
	void OnDemuxAudioThread(); // 录音解复用线程
	void OnSaveThread(); // 保存视频帧线程

	bool InitVideoOutput();
	bool InitAudioOutput();

	wchar_t* GetMicrophoneName();

private:
	

private:
	bool m_bRun = false;
	AVFormatContext* OutputFormatCtx = nullptr;
	AVFormatContext* AudioFormatCtx = nullptr;

	AVCodecContext* VideoCodecCtx = nullptr;
	AVCodecContext* AudioCodecCtx = nullptr;

	AVCodecContext* VideoEncCodecCtx = nullptr;
	AVCodecContext* AudioEncCodecCtx = nullptr;

	AVFrame* VideoFrame = nullptr;
	uint8_t* VideoBuffer = nullptr;
	AVFifoBuffer* VideoFifo = nullptr;

	AVAudioFifo* AudioFifo = nullptr;

	SwsContext* SwsCtx = nullptr;
	SwrContext* SwrCtx = nullptr;

	CCapAudio m_audioDecoder;
	CVideoDecoder m_videoDecoder;

	int VideoIndex = -1;
	int AudioIndex = -1;
	int StreamFrameRate = 0; // 刷新频率

	int m_nbSamples = 0;
	int m_nImageSize = 0;

	int64_t m_vCurPts = 0;
	int64_t m_aCurPts = 0;

	AVState m_state; // 

	int CapX, CapY, capWidth, capHeight;
	std::string m_szFilename;

	std::queue<AVFrame*> m_videoQueue;
	std::queue<AVFrame*> m_audioQueue;

	std::thread m_demuxVThread;
	std::thread m_demuxAThread;
	std::thread m_saveThread;
	std::mutex m_mutexPause;
	std::mutex m_mutexVideoBuf;
	std::mutex m_mutexAudioBuf;

	std::condition_variable m_cvPause;
	std::condition_variable m_cvVideoBufNotFull;
	std::condition_variable m_cvVideoBufNotEmpty;
	std::condition_variable m_cvAudioBufNotFull;
	std::condition_variable m_cvAudioBufNotEmpty;
};

