#pragma once
#include "Common.h"
#include "VideoEncoder.h"

class CAudioEncoder
{
public:
	CAudioEncoder();
	~CAudioEncoder();

	bool InitAudio(AVFormatContext* formatCtx, AVCodecID codecId);

	void Start(IEncoderEvent* pEvt);

	void Release();

	AVRational GetTimeBase();

	bool PushFrame(AVFrame* frame);

	int GetIndex();

private:
	void OnWork();

	bool PushFrameToFifo(AVFrame* frame);

	bool ReadPacketFromFifo();

	AVFrame* AllocOutputFrame(int nbSize);

private:
	IEncoderEvent* m_pEvent = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	AVStream* m_pStream = nullptr;

	AVAudioFifo* m_pAudioFifo = nullptr;
	uint8_t** m_convertBuffer = nullptr;

	int m_nbSamples = 0;

	SafeQueue<AVFrame*> m_pAudioQueue;
	bool m_bFinished = false;
	int m_frameIndex = 0;

	bool m_bRun = false;
	std::thread m_thread;
};

