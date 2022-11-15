#pragma once
#include "Common.h"
#include "PcmPlay.h"

class DemuxAudio
{
public:
	~DemuxAudio();

	bool Open(const char* szinput);

	bool SetOutput(const char* szOutput);

	void Run();

	void Release();

private:


private:
	AVFormatContext* InputFormatCtx = nullptr;
	AVCodecContext* InputCodecCtx = nullptr;
	AVFormatContext* OutputFormatCtx = nullptr;
	AVCodecContext* OutputCodecCtx = nullptr;

	SwrContext* SwrCtx = nullptr;
	AVPacket* InputPkt = nullptr;
	AVFrame* InputFrame = nullptr;

	DSPlayer	AudioPlayer;

	bool		m_bPlay = false;
	bool		m_bRecord = false;




	int AudioIndex = -1;
};

