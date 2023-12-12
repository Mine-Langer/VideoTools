#pragma once
#include "ffmpeg.h"
#include "ListAVDevice.h"
#include <string>
#include <cstdint>
#include <atomic>
#include <thread>
#include <cassert>

class AudioRecorder
{
public:
	AudioRecorder(std::string filePath, std::string device);
	~AudioRecorder();

	void Open();

	void Start();

	void Stop();

	std::string GetLastError();

private:
	void StartEncode();

private:
	std::string outFile = "";
	std::string deviceName = "";
	std::string failReason = "";

	AVFormatContext* audioInFormatCtx = nullptr;
	AVStream* audioInStream = nullptr;
	AVCodecContext* audioInCodecCtx = nullptr;

	SwrContext* audioConverter = nullptr;
	AVAudioFifo* audioFifo = nullptr;

	AVFormatContext* audioOutFormatCtx = nullptr;
	AVStream* audioOutStream = nullptr;
	AVCodecContext* audioOutCodecCtx = nullptr;

	std::atomic_bool isRun = false;
	std::thread audioThread;
	ListAVDevice listAVDevice;
};

