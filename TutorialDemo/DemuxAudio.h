#pragma once
#include "Common.h"
#include "PcmPlay.h"

class DemuxAudio
{
public:
	~DemuxAudio();

	bool Open(const char* szinput);


	void Run();

	void Release();

private:


private:
	AVFormatContext* InputFormatCtx = nullptr;
	AVCodecContext* InputCodecCtx = nullptr;
	SwrContext* SwrCtx = nullptr;
	AVPacket* InputPkt = nullptr;
	AVFrame* InputFrame = nullptr;

	DSPlayer	AudioPlayer;

	int AudioIndex = -1;
};

