#pragma once
#include "Common.h"

class CAudioEncoder
{
public:
	CAudioEncoder();
	~CAudioEncoder();

	bool InitAudio(AVFormatContext* formatCtx, AVCodecID codecId);

	void BindAudioData(SafeQueue<AVFrame*> *audioQueue);

	void Release();

	bool GetEncodePacket(AVPacket* pkt, int& aIndex);

	AVRational GetTimeBase();

	bool PushFrameToFifo(AVFrame* frameData, int framesize);

	AVPacket* GetPacketFromFifo(int* aIdx);

private:
	bool PushAudioToFifo();

	bool ReadPacketFromFifo(AVPacket* pkt);

	AVFrame* AllocOutputFrame(int nbSize);

private:
	AVCodecContext* m_pCodecCtx = nullptr;
	AVStream* m_pStream = nullptr;

	AVAudioFifo* m_pAudioFifo = nullptr;
	uint8_t** m_convertBuffer = nullptr;

	int m_nbSamples = 0;

	SafeQueue<AVFrame*>* m_pAudioQueue = nullptr;
	bool m_bFinished = false;
	int m_frameIndex = 0;
};

