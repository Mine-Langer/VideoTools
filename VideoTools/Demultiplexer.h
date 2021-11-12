#pragma once
#include "Common.h"

class CDemultiplexer
{
public:
	CDemultiplexer();

	~CDemultiplexer();

	bool Open(const char* szInput);

	void Start(IDemuxEvent* pEvt);

	void Release();

	int AudioStreamIndex();

	int VideoStreamIndex();

	AVFormatContext* FormatContext();

private:
	void OnDemuxFunction();

private:
	AVFormatContext* m_pFormatCtx = nullptr;

	int m_videoIndex = -1;
	int m_audioIndex = -1;

	bool m_bRun = false;
	std::thread m_thread;

	IDemuxEvent* m_pEvent = nullptr;
};

