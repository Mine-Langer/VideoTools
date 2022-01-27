#pragma once
#include "Demultiplex.h"

class CPlayer :public IDemuxEvent
{
public:
	CPlayer();
	~CPlayer();

	bool Open(const char* szFile);

	bool InitWindow(const void* pwnd, int width, int height);

	bool InitAudio();

private:
	void OnReadFunction();
	void OnPlayFunction();

	virtual bool OnDemuxPacket(AVPacket* pkt, int type) override;

	static void OnAudioCallback(void* userdata, Uint8* stream, int len);

private:
	CDemultiplex m_demux;
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
// 	CAVSync	m_sync;

	SDL_Window* m_window = nullptr;
	SDL_Texture* m_texture = nullptr;
	SDL_Renderer* m_render = nullptr;
	SDL_AudioSpec m_audioSpec;
	SDL_Rect m_rect;
};

