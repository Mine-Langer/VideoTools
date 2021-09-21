#pragma once
#include "CVideoDecoder.h"
#include "CAudioDecoder.h"

class CPlayer
{
public:
	CPlayer();
	~CPlayer();

	bool Open(const char* szFile);

	void Start();

private:
	void OnReadFunction();

private:
	bool m_bRun = false;
	AVFormatContext* FormatCtx = nullptr;

	int VideoIndex = -1;
	int AudioIndex = -1;

	std::thread m_ReadThread;

	CVideoDecoder m_videoDecoder;
	CAudioDecoder m_audioDecoder;

};

