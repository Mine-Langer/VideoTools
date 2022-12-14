#pragma once
#include "common.h"
#include "SafeQueue.h"


class Decoder
{
public:
	bool Init(AVFormatContext* fmt_ctx, int idx);

	AVFrame* PushPkt(AVPacket* pkt);

private:
	AVCodecContext* input_codec_ctx = nullptr;
	AVFrame* src_frame = nullptr;
	SwrContext* swr_ctx = nullptr;

	AVChannelLayout out_ch_layout;
	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_FLTP;
	int out_sample_rate = 44100;
};


class Encoder
{
public:
	bool Init();

	void writeHeader();

	void writeTail();

	void writeFrame(bool bFinished);

	void PushFrame(AVFrame* frame);

private:
	AVFormatContext* output_fmt_ctx = nullptr;
	AVCodecContext* output_codec_ctx = nullptr;
	AVAudioFifo* fifo = nullptr;

	AVPacket* dst_pkt = nullptr;

	int64_t _pts = 0;
};


class CDemux
{
public:
	bool Open(const char* szinput);

private:
	void OnDemux();

private:
	AVFormatContext* input_fmt_ctx = nullptr;
	
	Decoder decoder;

	Encoder encoder;

	int video_index = -1;
	int audio_index = -1;
};


class ConvertDemo
{
public:
	bool Save(const char* szOut, const char* szInput);

private:
	bool OpenInput(const char* szInput);

	bool OpenOutput(const char* szOutput);

	void Release();

	bool InitConfig();

	void dowork();

	void PushFrameToFifo(AVFrame* frame);
	void PopFrameToEncodeAndWrite(bool bFinished = false);


private:
	AVFormatContext* input_fmt_ctx = nullptr;
	AVFormatContext* output_fmt_ctx = nullptr;
	AVCodecContext* input_codec_ctx = nullptr;
	AVCodecContext* output_codec_ctx = nullptr;
	SwrContext* swr_ctx = nullptr;
	AVAudioFifo* fifo = nullptr;
	AVPacket* input_packet = nullptr;
	AVFrame* input_frame = nullptr;
	AVPacket* output_packet = nullptr;

	int audio_index = -1;
	int64_t _pts = 0;
};

