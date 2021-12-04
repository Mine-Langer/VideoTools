#pragma once
#include "../AudioDecoder.h"
#include "../VideoDecoder.h"
#include "../AudioEncoder.h"
#include "../VideoEncoder.h"

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

	void Close();

	void OnSaveThread(); // 保存视频帧线程

	wchar_t* GetMicrophoneName();

private:
	

private:
	bool m_bRun = false;
	AVFormatContext* OutputFormatCtx = nullptr;

	CAudioDecoder m_audioDecoder;
	CVideoDecoder m_videoDecoder;
	CAudioEncoder m_audioEncoder;
	CVideoEncoder m_videoEncoder;

	SafeQueue<AVFrame*> m_videoQueue;
	SafeQueue<AVFrame*> m_audioQueue;

	int VideoIndex = -1;
	int AudioIndex = -1;
	int StreamFrameRate = 0; // 刷新频率

	int m_nbSamples = 0;
	int m_nImageSize = 0;

	AVState m_state; // 

	int CapX, CapY, capWidth, capHeight;
	std::string m_szFilename;

	std::thread m_thread;
};

