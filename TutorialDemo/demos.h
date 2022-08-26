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