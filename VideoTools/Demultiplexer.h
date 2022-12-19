#pragma once
#include "Common.h"

class IDemuxEvent
{
public:
	virtual bool DemuxPacket(AVPacket* pkt, int type) = 0;

	virtual void CleanPacket() = 0;
};

class CDemultiplexer
{
public:
	CDemultiplexer();

	~CDemultiplexer();

	// 读取文件
	bool Open(const char* szInput);

	// 读取设备
	bool Open(const char* szInput, const AVInputFormat* ifmt, AVDictionary** pDict);

	void Start(IDemuxEvent* pEvt);

	void Stop();

	void Release();

	void WaitFinished();

	void SetPosition(int64_t dwTime);

	void SetUniqueStream(bool bUnique, int uniqueStream);

	int AudioStreamIndex();

	int VideoStreamIndex();

	AVFormatContext* FormatContext();

private:
	void OnDemuxFunction();

private:
	AVFormatContext* m_pFormatCtx = nullptr;

	int m_videoIndex = -1;
	int m_audioIndex = -1;
	int m_uniqueIndex = -1;

	bool m_bRun = false;
	std::thread m_thread;

	bool m_bUniqueStream = false;
	bool m_seek = false;
	int64_t m_target_pts = 0;
	IDemuxEvent* m_pEvent = nullptr;
};

