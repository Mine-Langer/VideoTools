#pragma once
#include "CVideoDecoder.h"

class CRecorder
{
public:
	CRecorder();
	~CRecorder();

	bool Run();

	// 初始化录制视频
	bool InitVideo();

private:
	AVFormatContext* VideoFormatCtx = nullptr;

	int VideoIndex = -1;

	CVideoDecoder m_videoDecoder;
};

