#pragma once
#include "ffmpeg.h"

class AudioTranscode
{
public:
	void Run();

	bool open_input_file(const char* filename);

	bool open_output_file(const char* filename);

	bool init_resampler();

	bool init_fifo();

	bool write_output_file_header();

private:
	void OnWork();

	bool readDecodeConvertAndStore(int* finished);

	bool decodeAudioFrame(AVFrame* frame, int* data_present, int* finished);

	bool loadEncodeAndWrite();

	bool encodeAudioFrame(AVFrame* frame, int* data_present);

	void clean();

private:
	AVFormatContext* input_format_context = NULL, * output_format_context = NULL;
	AVCodecContext* input_codec_context = NULL, * output_codec_context = NULL;
	SwrContext* resample_context = NULL;
	AVAudioFifo* fifo = NULL;

	int64_t pts = 0;
};

