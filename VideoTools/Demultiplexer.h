#pragma once
#include "Common.h"

class IDemuxEvent
{
public:
	virtual bool DemuxPacket(AVPacket* pkt, int type) = 0;
};

class CDemultiplexer
{
public:
	CDemultiplexer();

	~CDemultiplexer();

	// 读取文件
	bool Open(const char* szInput);

	// 读取设备
	bool Open(const char* szInput, AVInputFormat* ifmt, AVDictionary** pDict);

	void Start(IDemuxEvent* pEvt);

	void Stop();

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

