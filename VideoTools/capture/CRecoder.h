#pragma once
#include "../AudioDecoder.h"
#include "../AudioEncoder.h"
#include "../VideoEncoder.h"
#include "captureScreen.h"

enum eAudioOpt { AllCap, SysAudio, MicroAudio, NoAudio };

class CRecoder :public IDecoderEvent
{
public:
	CRecoder();
	~CRecoder();

	void SetVideoOption(int x, int y, int w, int h);

	void SetAudioOption(enum eAudioOpt audioOpt);

	void SetSaveFile(const char* szName);

	bool Start();

	void Pause();

	void Stop();

private:
	void Close(); // �رղ��ͷ���Դ

	// ��ʼ�����
	bool InitOutput();

	// ��ʼ������
	bool InitInput();

	// ¼���߳�
	void CaptureThread();

	bool WriteHeader();

	void WriteTrailer();

	virtual bool VideoEvent(AVFrame* frame) override;

	virtual bool AudioEvent(AVFrame* frame) override;

private:
	AVState m_status = Stopped;
	eAudioOpt m_audioOption = NoAudio;
	int m_x = 0, m_y = 0, m_w = 0, m_h = 0;
	std::string m_szFile;
	std::thread m_thread;

	SafeQueue<AVFrame*> m_vDataQueue;
	SafeQueue<AVFrame*> m_aDataQueue;
	AVFrame* m_swsFrame = nullptr;

	AVFormatContext* m_pFormatCtx = nullptr;
//	CVideoDecoder m_videoDecoder;
	CVideoEncoder m_videoEncoder;
	CaptureScreen m_captureScreen;
};

/*
class CRecorder :public IVideoEvent, public IAudioEvent
{
public:
	CRecorder();
	~CRecorder();

	void InitVideoCfg(int posX, int posY, int sWidth, int sHeight);

	void InitAudioOption(eAudioOpt audioOpt);

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

	void Release();

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

	eAudioOpt m_audioOption;	// ��Ƶѡ��
	int CapX, CapY, capWidth, capHeight; // ��Ƶѡ��
	std::string m_szFilename;

	std::thread m_thread;
};
*/

