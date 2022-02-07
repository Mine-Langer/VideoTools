#pragma once
#include "Demultiplex.h"
#include "AudioDecoder.h"
#include "avAsync.h"

class IPlayerEvent
{
public:
	virtual void OnPlayStatus(eAVStatus eStatus) = 0;
};

class CPlayer :public IDemuxEvent, public IDecoderEvent
{
public:
	CPlayer();
	~CPlayer();

	bool Open(const char* szFile);

	bool InitWindow(const void* pwnd, int width, int height);

	void Start(IPlayerEvent* pEvent);

	void SetPosition(); // 设置位置

	void UpdateWindow(int width, int height);
	void UpdateWindow(int x, int y, int width, int height);

	// 关闭播放
	void Close();


private:
	// 释放资源
	void Release();

	bool InitAudio();

	void OnPlayFunction();

	virtual bool OnDemuxPacket(AVPacket* pkt, int type) override;
	virtual bool VideoEvent(AVFrame* vdata) override;
	virtual bool AudioEvent(AVFrame* adata) override;

	static void OnAudioCallback(void* userdata, Uint8* stream, int len);

private:
	CDemultiplex m_demux;
	CVideoDecoder m_videoDecoder;
	CAudioDecoder m_audioDecoder;

	eAVStatus m_avStatus = eStop; 
	int m_bPlayOvered = false;

// 	AVFormatContext* FormatCtx = nullptr;
// 
// 	AVPacket* SrcPacket = nullptr;
// 
// 	int VideoIndex = -1;
// 	int AudioIndex = -1;
// 
// 	SafeQueue<AVFrame*> VideoFrameData;
// 	SafeQueue<STAudioBuffer*> AudioFrameData;
// 
// 	std::thread m_ReadThread;
// 	std::thread m_PlayThread;
// 	IPlayEvent* m_playEvent = nullptr;
// 
// 	CVideoDecoder m_videoDecoder;
// 	CAudioDecoder m_audioDecoder;
 	CAVSync	m_sync;

	SDL_Window* m_window = nullptr;
	SDL_Texture* m_texture = nullptr;
	SDL_Renderer* m_render = nullptr;
	SDL_AudioSpec m_audioSpec;
	SDL_Rect m_rect;

	SafeQueue<AVFrame*> m_videoQueue;
	SafeQueue<AVFrame*> m_audioQueue;

	IPlayerEvent* m_pEvent = nullptr;

	std::thread m_playThread;
};

