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

	void Pause(); // ��ͣ

	void Resume(); // ����

	void Stop(); // ��ֹ

protected:
	virtual bool VideoEvent(AVFrame* frame) override;
	virtual bool AudioEvent(AVFrame* frame) override;

private:
	bool InitOutput(const char* szOutput);

	bool Start(); // �����⸴��

	void Close();

	void OnSaveThread(); // ������Ƶ֡�߳�

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
	int StreamFrameRate = 0; // ˢ��Ƶ��

	int m_nbSamples = 0;
	int m_nImageSize = 0;

	AVState m_state; // 

	int CapX, CapY, capWidth, capHeight;
	std::string m_szFilename;

	std::thread m_thread;
};

