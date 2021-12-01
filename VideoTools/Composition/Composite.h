#pragma once
#include "../VideoDecoder.h"
#include "../AudioDecoder.h"
#include "../VideoEncoder.h"
#include "../AudioEncoder.h"
#include "../FilterVideo.h"

#define E_Play 0x1
#define E_Save 0x2
class Composite :public IVideoEvent, public IAudioEvent
{
public:
	Composite();
	~Composite();

	void AddAudio(const char* szFile);
	void AddImage(const char* szFile);

	void Start();

	void Close();

	void Play(); // 播放

	// 设置播放时的显示参数
	bool InitWnd(void* pWnd, int width, int height);

	// 保存文件
	bool SaveFile(const char* szOutput, int type);

private:
	virtual bool VideoEvent(AVFrame* frame) override;
	virtual bool AudioEvent(AVFrame* frame) override;

private:
	bool OpenAudio(const char* szFile);
	bool OpenImage(const char* szFile);

	bool InitAudioEnc(enum AVCodecID codec_id);

private:
	static void OnSDLAudioFunction(void* userdata, Uint8* stream, int len);
	void OnPlayFunction(); // 
	void OnSaveFunction(); // 保存文件

private:
	CVideoDecoder m_videoDecoder;
	CAudioDecoder m_audioDecoder;
	CVideoEncoder m_videoEncoder;
	CAudioEncoder m_audioEncoder;

	SafeQueue<AVFrame*> m_videoQueue;
	SafeQueue<AVFrame*> m_audioQueue;

	AVFormatContext* m_pOutFormatCtx = nullptr;
	CFilterVideo m_filter;

	// 播放
	SDL_AudioSpec m_audioSpec;
	SDL_Window* m_window = nullptr;
	SDL_Texture* m_texture = nullptr;
	SDL_Renderer* m_render = nullptr;
	SDL_Rect m_rect;

	int m_videoWidth = 0;
	int m_videoHeight = 0;

	int m_bitRate = 0;
	int m_frameRate = 0;
	int m_audioFrameSize = 0;

	std::thread m_playThread;
	std::thread m_saveThread;
	AVState m_state = NotStarted;
	int m_type = 0; // 0: 预览播放  1：保存文件
	char m_szAudioFile[128] = { 0 };
	char m_szVideoFile[128] = { 0 };
};

