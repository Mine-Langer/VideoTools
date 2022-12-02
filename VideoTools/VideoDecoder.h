#pragma once
#include "Demultiplexer.h"

enum AVState { NotStarted, Started, Paused, Stopped };

class IDecoderEvent
{
public:
	virtual bool VideoEvent(AVFrame* frame) = 0;

	virtual bool AudioEvent(AVFrame* frame) = 0;
};

class CVideoDecoder //:public IDemuxEvent
{
public:
	CVideoDecoder();
	virtual ~CVideoDecoder();

	// 打开输入文件
	bool Open(const char* szInput);
	
	bool Open(CDemultiplexer* pDemux);

	// 打开屏幕录像
	bool OpenScreen(int posX, int posY, int sWidth, int sHeight);
	// 打开摄像头
	bool OpenCamera();

	bool Start(IDecoderEvent* pEvt);

	void Stop();

	bool SendPacket(AVPacket* pkt);

	bool SetSwsConfig(SDL_Rect* rect = nullptr, int width = -1, int height = -1, enum AVPixelFormat pix_fmt = AV_PIX_FMT_NONE);
	void GetSrcParameter(int& srcWidth, int& srcHeight, enum AVPixelFormat& srcFormat);

	AVFrame* ConvertFrame(AVFrame* frame);
	double Timebase();

protected:
	void OnDecodeFunction();

	void Release();

private:
	//CDemultiplexer m_demux;
	AVCodecContext* m_pCodecCtx = nullptr;
	SwsContext* m_pSwsCtx = nullptr;
	IDecoderEvent* m_pEvent = nullptr;

	bool m_bRun = false;
	enum AVState m_state = NotStarted;
	std::thread m_thread;

	double m_timebase = 0.0;
	int m_swsWidth = 0, m_srcWidth = 0;
	int m_swsHeight = 0, m_srcHeight = 0;
	enum AVPixelFormat m_swsFormat = AV_PIX_FMT_NONE, m_srcFormat = AV_PIX_FMT_NONE;
	SafeQueue<AVPacket*> m_srcVPktQueue;
};

