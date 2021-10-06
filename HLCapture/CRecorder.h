#pragma once
#include "CVideoDecoder.h"

class CRecorder
{
public:
	CRecorder();
	~CRecorder();

	bool Run();

	// ��ʼ��¼����Ƶ
	bool InitVideo();

private:
	AVFormatContext* VideoFormatCtx = nullptr;

	int VideoIndex = -1;

	CVideoDecoder m_videoDecoder;
};

