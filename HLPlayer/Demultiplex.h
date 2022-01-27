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

	// ���ļ�
	bool Open(const char* szFile);

	// ����
	void Start(IDemuxEvent* pEvent);

	// �ر�
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

	eAVStatus m_avStatus = eStop; // Ĭ��Ϊ��ֹ״̬
};

