#pragma once
#include "../Common.h"


class CCapVideo
{
public:
	CCapVideo();
	~CCapVideo();

	bool Init(int posX, int posY, int sWidth, int sHeight);

	// ≥ı ºªØ…„œÒÕ∑
	bool InitCamera();

	bool Start(IVideoEvent* pEvt);

	void Release();

private:
	void OnDecodeFunction();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	SwsContext* m_swsCtx = nullptr;

	enum RecordState m_state = NotStarted;

	int m_videoIndex = -1;
	int m_imageWidth = 0;
	int m_imageHeight = 0;

	IVideoEvent* m_pEvent = nullptr;

	std::thread m_thread;
};

