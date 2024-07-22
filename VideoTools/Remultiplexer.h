#pragma once
#include "Common.h"

// 视频封包相关回调接口
class IRemuxEvent
{
public:
	virtual void RemuxEvent(AVPacket* pkt, int nType, int64_t pts) = 0;
};

class ITranscodeProgress
{
public:
	virtual void ProgressValue(int second_time) = 0;
};


/*
* 重封装复用器
*/
class CRemultiplexer : public IRemuxEvent
{
public:
	CRemultiplexer();
	~CRemultiplexer();

	bool Open(const std::string& strFile, bool aEnable = true, bool vEnable = false, int w=640, int h=480);

	void Start(ITranscodeProgress* pEvt);

	void Close();

	void SetAudioResampler();


	void SendAudioFrame(uint8_t* pBuf, int BufSize);
	void SendAudioFrame(CAVFrame* frame);
	void SendAudioFrame(AVFrame* frame);
	void SendVideoFrame(AVFrame* frame);

protected:
	virtual void RemuxEvent(AVPacket* pkt, int nType, int64_t pts) override;

protected:
	void Work();

	void Init();

	void Cleanup();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	ITranscodeProgress* m_pTransEvent = nullptr;

	class CAudioEncoder* m_pAudioEncode = nullptr;
	class CVideoEncoder* m_pImageEncode = nullptr;
	AVStream* m_pAudioStream = nullptr;
	AVStream* m_pVideoStream = nullptr;

	AVRational m_AudioTimeBase;
	AVRational m_VideoTimeBase;

	SafeQueue<CAVFrame*> m_AudioData;
	SafeQueue<CAVFrame*> m_VideoData;

	SafeQueue<AVPacket*> m_AudioPacket;
	SafeQueue<AVPacket*> m_videoPacket;

	bool m_bRun = false;
	bool m_AudioEnable = false;
	bool m_VideoEnable = false;
	std::thread m_thread;

	// 视频参数
	int		m_FPS = 25;
	int		m_VideoBitrate = 2000000;
	//int		m_Width = 640;
	//int		m_Height = 480;
	int		m_AudioBitrate = 128000;
};

