#pragma once
#include "Demultiplexer.h"

class CVideoDecoder :public IDemuxEvent
{
public:
	CVideoDecoder();
	virtual ~CVideoDecoder();

	// �������ļ�
	bool Open(const char* szInput);
	// ����Ļ¼��
	bool OpenScreen(int posX, int posY, int sWidth, int sHeight);
	// ������ͷ
	bool OpenCamera();

	bool Start(IVideoEvent* pEvt);

	void Stop();

	void Release();

	bool SetSwsConfig(int width = -1, int height = -1, enum AVPixelFormat pix_fmt = AV_PIX_FMT_NONE);
	void GetSrcParameter(int& srcWidth, int& srcHeight, enum AVPixelFormat& srcFormat);

	void SetVideoInfo(int x, int y, int width, int height);

	AVFrame* GetConvertFrame(AVFrame* frame);

protected:
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;
	void OnDecodeFunction();

	void Close();

private:
	CDemultiplexer m_demux;
	AVCodecContext* m_pCodecCtx = nullptr;
	SwsContext* m_pSwsCtx = nullptr;
	IVideoEvent* m_pEvent = nullptr;

	enum AVState m_state = NotStarted;
	std::thread m_thread;

	int m_swsWidth = 0, m_srcWidth = 0;
	int m_swsHeight = 0, m_srcHeight = 0;
	enum AVPixelFormat m_swsFormat = AV_PIX_FMT_NONE, m_srcFormat = AV_PIX_FMT_NONE;
	SafeQueue<AVPacket*> m_srcVPktQueue;
};

