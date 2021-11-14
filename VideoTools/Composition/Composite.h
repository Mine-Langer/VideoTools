#pragma once
#include "CompositeVideo.h"
#include "CompositeAudio.h"

class Composite :public IVideoEvent, public IAudioEvent
{
public:
	Composite();
	~Composite();

	bool OpenImage(const char* szFile);

	bool OpenAudio(const char* szFile);

	void Start();

	void Close();

	void Play(); // 播放

	// 设置播放时的显示参数
	bool InitWnd(void* pWnd, int width, int height);

	// 保存文件
	bool SaveFile(const char* szOutput);

private:
	virtual bool VideoEvent(AVFrame* frame) override;
	virtual bool AudioEvent(AVFrame* frame) override;

private:
	void InitVideoEnc(enum AVCodecID codec_id);
	void InitAudioEnc(enum AVCodecID codec_id);

private:
	static void OnSDLAudioFunction(void* userdata, Uint8* stream, int len);
	void OnPlayFunction(); // 
	void OnSaveFunction(); // 保存文件

private:
	CompositeVideo m_videoDecoder;
	CompositeAudio m_audioDecoder;
	std::queue<AVFrame*> m_videoQueue;
	std::queue<AVFrame*> m_audioQueue;

	AVFormatContext* m_pOutFormatCtx = nullptr;

	// 播放
	SDL_AudioSpec m_audioSpec;
	SDL_Window* m_window = nullptr;
	SDL_Texture* m_texture = nullptr;
	SDL_Renderer* m_render = nullptr;
	SDL_Rect m_rect;

	int m_videoWidth = 0;
	int m_videoHeight = 0;

	std::thread m_playThread;
	std::thread m_saveThread;
	RecordState m_state = NotStarted;
};

