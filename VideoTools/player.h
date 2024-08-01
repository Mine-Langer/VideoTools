#pragma once

// #include "DxAudioPlayer.h"
// #include "FilterVideo.h"
// #include "avSync.h"
#include "Common.h"

class CPlayer 
{
public:
	CPlayer();
	~CPlayer();

	bool Open(const char* szInput);

	void SetView(HWND hWnd, int w, int h);

	void SetAudioSpec(int sample_rate, AVChannelLayout ch_layout, int samples);

	void StartPlay();
	void StartRender();

	void Stop();

	void Pause();

	void Seek(uint64_t pts_time);

	void Release();

	void SendAudioFrame(AVFrame* frame);
	void SendVideoFrame(AVFrame* frame);

	void CalcImageView(SDL_Rect rect, AVFrame* frame);

protected:
	// 计算显示矩形区域
	void CalcDisplayRect(int x, int y, int scrWidth, int scrHeight, int picWidth, int picHeight, AVRational pic_sar);


private:	 
	void OnRenderProc(); // 渲染图像线程

	// 播放音频回调函数
	static void	OnAudioCallback(void* userdata, Uint8* stream, int len);


private:

private:
// 	CDemultiplexer	m_demux;
// 	CAudioDecoder	m_audioDecoder;
// 	CVideoDecoder	m_videoDecoder;
// 	CFilterVideo	m_filter;

	SDL_Window*		m_pWindow = nullptr;
	SDL_Renderer*	m_pRender = nullptr;
	SDL_Texture*	m_pTexture = nullptr;
	SDL_Rect		m_rect;
	SDL_AudioSpec	m_audioSpec;
	AVSync			m_avSync;

	int m_scrWidth;
	int m_scrHeight;

	SafeQueue<AVFrame*> m_audioFrameQueue;
	SafeQueue<AVFrame*> m_videoFrameQueue;

	bool m_bRun = false;
	bool m_pause = false;

	std::thread m_tPlay;
	std::thread m_tRender;

};

