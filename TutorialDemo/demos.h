#pragma once
#include "Common.h"
#include "PcmPlay.h"
#define _CRT_SECURE_NO_WARNINGS 1 

struct buffer_data
{
	uint8_t* ptr;
	size_t size;
};

class CDemos
{
public:
	~CDemos();

	void ExtractMvs(const char* szFile);

	void AvioReading(const char* szFile);

private:
	static int read_packet(void* opaque, uint8_t* buf, int buf_size);

private:
	AVFormatContext* fmt_ctx = nullptr;
	AVIOContext* avio_ctx = nullptr;

	uint8_t* buffer = nullptr;
	uint8_t* avio_ctx_buffer = nullptr;
	size_t buffer_size, avio_ctx_buffer_size = 4096;
	buffer_data buf_data = { 0 };

};

class ExtractMvs
{
public:
	ExtractMvs(const char* szFile);
	~ExtractMvs();

private:
	bool Open(const char* szFile);
	void Run();
	bool DecodePacket(AVPacket* pkt);

private:
	AVFormatContext* _format_ctx = nullptr;
	AVCodecContext* _video_codec_ctx = nullptr;
	AVStream* _video_stream = nullptr;
	AVFrame* _frame = nullptr;
	int stream_video_index = -1;
	int video_frame_count = 0;
};


/***************************************************************/
class CTransAAC
{
public:
	~CTransAAC();

	bool Run(const char* szInput, const char* szOutput);

private:
	bool OpenInput(const char* szInput);

	bool OpenOutput(const char* szOutput);

	void Release();

	bool InitConfig();

	void dowork();

	void PushFrameToFifo(const uint8_t** framedata, int framesize);
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

class CAudioConvert
{
public:
	virtual ~CAudioConvert();

	bool Open(const char* szfile);

	bool Save(const char* szfile);

	void SetOption(int nChannel, int bitRate);

	void Start();

	void Close();

private:
	bool SetOutputOpt();

private:
	AVFormatContext *InputFormatCtx = nullptr, *OutputFormatCtx = nullptr;
	AVCodecContext *InputCodecCtx = nullptr, *OutputCodecCtx = nullptr;

	SwrContext* SwrCtx = nullptr;
	AVAudioFifo* Fifo = nullptr;

	AVFrame *SrcFrame = nullptr, *DestFrame = nullptr;
	AVPacket *SrcPacket = nullptr, *DestPacket = nullptr;

	int AudioIndex = -1;

	int OutputBitRate = 96000;
	int OutputChannels = 2;

};

class CFilterAudio
{
public:
#define INPUT_SAMPLERATE     48000
#define INPUT_FORMAT         AV_SAMPLE_FMT_FLTP
#define INPUT_CHANNEL_LAYOUT (AVChannelLayout)AV_CHANNEL_LAYOUT_5POINT0
#define VOLUME_VAL			0.90
#define FRAME_SIZE			1024

	void Run(float fDuration);

private:
	bool InitFilterGraph();

private:
	AVMD5* _md5 = nullptr;
	AVFilterGraph* _graph = nullptr;
	AVFilterContext* _src_ctx = nullptr;
	AVFilterContext* _sink_ctx = nullptr;
	AVFrame* _frame = nullptr;
};

int OnFilterAudio(const char* szDur);

int OnFilteringAudio(const char* szInput);