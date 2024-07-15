#pragma once
#include "Common.h"

// ��Ƶ�����ػص��ӿ�
class IRemuxEvent
{
public:
	virtual void RemuxEvent(AVPacket* pkt, int nType, int64_t pts) = 0;
};

/*
* �ط�װ������
*/
class CRemultiplexer : public IRemuxEvent
{
public:
	CRemultiplexer();
	~CRemultiplexer();

	bool Open(const std::string& strFile);

	void Start();

	void Close();

	void SetAudioResampler();


	void SendAudioFrame(uint8_t* pBuf, int BufSize);
	void SendAudioFrame(CAVFrame* frame);
	void SendAudioFrame(AVFrame* frame);
	void SendVideoFrame(CAVFrame* frame);

protected:
	virtual void RemuxEvent(AVPacket* pkt, int nType, int64_t pts) override;

protected:
	void Work();

	void Init();

	void Cleanup();

private:
	AVFormatContext* m_pFormatCtx = nullptr;

	class CAudioEncoder* m_pAudioEncode = nullptr;
	class CVideoEncoder* m_pImageEncode = nullptr;
	AVStream* m_pAudioStream = nullptr;
	AVStream* m_pVideoStream = nullptr;

	AVRational m_AudioTimeBase;
	AVRational m_VideoTimeBase;

	SafeQueue<CAVFrame*> m_AudioData;
	SafeQueue<CAVFrame*> m_VideoData;

	SafeQueue<AVPacket*> m_AudioPacket;

	bool m_bRun = false;
	bool m_AudioEnable = false;
	bool m_VideoEnable = false;
	std::thread m_thread;

	// ��Ƶ����
	int		m_FPS = 25;
	int		m_VideoBitrate = 2000000;
	int		m_Width = 640;
	int		m_Height = 480;
	int		m_AudioBitrate = 128000;
};

