#pragma once
#include "Common.h"

class IDemuxEvent
{
public:
	virtual bool OnDemuxPacket(AVPacket* pkt, int type) = 0;
};

class CDemultiplex
{
public:
	CDemultiplex();
	~CDemultiplex();

	// 打开文件
	bool Open(const char* szFile);

	// 播放
	void Start(IDemuxEvent* pEvent);

	// 关闭
	void Close();

private:
	void OnDemuxThread();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	IDemuxEvent* m_pDemuxEvent = nullptr;

	int m_audioIndex = -1;
	int m_videoIndex = -1;
	int m_subtitleIndex = -1;

	std::thread m_demuxThread;

	eAVStatus m_avStatus = eStop; // 默认为终止状态
};

