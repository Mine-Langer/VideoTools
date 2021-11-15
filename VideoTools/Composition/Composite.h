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

	void Play(); // ����

	// ���ò���ʱ����ʾ����
	bool InitWnd(void* pWnd, int width, int height);

	// �����ļ�
	bool SaveFile(const char* szOutput);

private:
	virtual bool VideoEvent(AVFrame* frame) override;
	virtual bool AudioEvent(AVFrame* frame) override;

private:
	bool InitVideoEnc(enum AVCodecID codec_id);
	bool InitAudioEnc(enum AVCodecID codec_id);

private:
	static void OnSDLAudioFunction(void* userdata, Uint8* stream, int len);
	void OnPlayFunction(); // 
	void OnSaveFunction(); // �����ļ�

private:
	CompositeVideo m_videoDecoder;
	CompositeAudio m_audioDecoder;
	SafeQueue<AVFrame*> m_videoQueue;
	SafeQueue<AVFrame*> m_audioQueue;

	AVFormatContext* m_pOutFormatCtx = nullptr;
	AVCodecContext* m_pOutVCodecCtx = nullptr;
	AVCodecContext* m_pOutACodecCtx = nullptr;
	AVStream* m_pOutVStream = nullptr;
	AVStream* m_pOutAStream = nullptr;
	SwrContext* m_pSwrCtx = nullptr;
	AVAudioFifo* m_pAudioFifo = nullptr;

	// ����
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
	RecordState m_state = NotStarted;
};

