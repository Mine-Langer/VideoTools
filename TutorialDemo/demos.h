#pragma once
#include "Common.h"

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

class TAAC
{
public:
	~TAAC();

	bool Run(const char* szInput, const char* szOutput);

private:
	bool OpenInput(const char* szInput);

	bool OpenOutput(const char* szOutput);

	bool InitSwr();

	bool InitFifo();

	bool ReadDecodeConvertStore(bool* finished);

	bool InitConvertSamples(int frameSize);

	bool LoadEncodeAndWrite();

	bool ConvertSamples(AVFrame* frame);

	bool AddSamplesToFifo(int nbSamples);
		 

	void Release();
	
private:
	AVFormatContext* input_fmt_ctx = nullptr;
	AVFormatContext* output_fmt_ctx = nullptr;
	AVCodecContext* input_codec_ctx = nullptr;
	AVCodecContext* output_codec_ctx = nullptr;
	SwrContext* swr_ctx = nullptr;
	AVAudioFifo* fifo = nullptr;
	uint8_t** converted_samples = nullptr;

	int audio_stream_idx = -1;
	int64_t _pts = 0;
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
	void PopFrameToEncodeAndWrite();



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