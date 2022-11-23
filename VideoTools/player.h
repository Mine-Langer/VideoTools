#pragma once
#include "Demultiplexer.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "DxAudioPlayer.h"

class CPlayer :public IDemuxEvent, public IDecoderEvent
{
public:
	CPlayer();
	~CPlayer();

	bool Open(const char* szInput);

	void SetView(HWND hWnd, int w, int h);

	void Start();

	void Stop();

	void Release();

private:
	void OnPlayProc();

private:
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;

	virtual bool VideoEvent(AVFrame* frame) override;

	virtual bool AudioEvent(AVFrame* frame) override;

private:
	CDemultiplexer	m_demux;
	CAudioDecoder	m_audioDecoder;
	CVideoDecoder	m_videoDecoder;
	DxAudioPlayer	m_dxAudio;

	SafeQueue<AVFrame*> m_audioFrameQueue;
	SafeQueue<AVFrame*> m_videoFrameQueue;

	bool m_bRun = false;

	std::thread m_tPlay;
};

