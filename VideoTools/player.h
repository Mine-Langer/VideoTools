#pragma once
#include "Demultiplexer.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "DxAudioPlayer.h"
#include "FilterVideo.h"
#include "avSync.h"

class CPlayer :public IDemuxEvent, public IDecoderEvent
{
public:
	CPlayer();
	~CPlayer();

	bool Open(const char* szInput);

	void SetView(HWND hWnd, int w, int h);

	void SetAudioSpec(int sample_rate, AVChannelLayout ch_layout, int samples);

	void Start();

	void Stop();

	void Pause();

	void Seek(uint64_t pts_time);

	void Release();

	void SendAudioFrame(AVFrame* frame);
	void SendVideoFrame(AVFrame* frame);

	void PlayAudio();

private:
	void OnPlayProc();
	void OnRenderProc();

	static void	OnAudioCallback(void* userdata, Uint8* stream, int len);


private:
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;

	virtual void CleanPacket() override;

	virtual bool VideoEvent(AVFrame* frame) override;

	virtual bool AudioEvent(AVFrame* frame) override;

private:
	CDemultiplexer	m_demux;
	CAudioDecoder	m_audioDecoder;
	CVideoDecoder	m_videoDecoder;
	DxAudioPlayer	m_dxAudio;
	CFilterVideo	m_filter;

	SDL_Window*		m_pWindow = nullptr;
	SDL_Renderer*	m_pRender = nullptr;
	SDL_Texture*	m_pTexture = nullptr;
	SDL_Rect		m_rect;
	SDL_AudioSpec	m_audioSpec;
	AVSync			m_avSync;

	SafeQueue<AVFrame*> m_audioFrameQueue;
	SafeQueue<AVFrame*> m_videoFrameQueue;

	bool m_bRun = false;
	bool m_pause = false;

	std::thread m_tPlay;
	std::thread m_tRender;
};

