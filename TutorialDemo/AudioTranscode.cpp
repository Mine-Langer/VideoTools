#include "AudioTranscode.h"

#define OUTPUT_BIT_RATE 96000
#define OUTPUT_CHANNELS 2

void AudioTranscode::Run()
{
	if (!open_input_file("F:/CloudMusic/Paradox Interactive - The Titan.mp3"))
		return;

	if (!open_output_file("output.aac"))
		return;

	if (!init_resampler())
		return;

	if (!init_fifo())
		return;

	if (!write_output_file_header())
		return;
	
	OnWork();

	clean();
}

bool AudioTranscode::open_input_file(const char* filename)
{
	/* Open the input file to read from it. */
	if (0 > avformat_open_input(&input_format_context, filename, nullptr, nullptr)) {
		return false;
	}

	/* Get information on the input file (number of streams etc.). */
	if (0 > avformat_find_stream_info(input_format_context, nullptr)) {
		return false;
	}

	const AVCodec* input_codec = nullptr;
	int idx = av_find_best_stream(input_format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &input_codec, 0);
	if (0 > idx)
		return false;

	const AVStream* stream = input_format_context->streams[idx];

	/* Allocate a new decoding context. */
	input_codec_context = avcodec_alloc_context3(input_codec);
	if (!input_codec_context) {
		return false;
	}

	/* Initialize the stream parameters with demuxer information. */
	if (0 > avcodec_parameters_to_context(input_codec_context, stream->codecpar)) {
		return false;
	}

	/* Open the decoder for the audio stream to use it later. */
	if (0 > avcodec_open2(input_codec_context, input_codec, NULL)) {
		return false;
	}

	/* Set the packet timebase for the decoder. */
	input_codec_context->pkt_timebase = stream->time_base;
	
	return true;
}

bool AudioTranscode::open_output_file(const char* filename)
{
	if (0 > avformat_alloc_output_context2(&output_format_context, nullptr, nullptr, filename))
		return false;

	if (0 > avio_open(&output_format_context->pb, filename, AVIO_FLAG_WRITE))
		return false;

	const AVOutputFormat* output_format = output_format_context->oformat;

	const AVCodec* output_codec = nullptr;
	if (!(output_codec = avcodec_find_encoder(output_format->audio_codec)))
		return false;

	AVStream* stream = avformat_new_stream(output_format_context, nullptr);
	if (!stream)
		return false;

	output_codec_context = avcodec_alloc_context3(output_codec);
	if (!output_codec_context)
		return false;

	av_channel_layout_default(&output_codec_context->ch_layout, OUTPUT_CHANNELS);
	output_codec_context->sample_rate = input_codec_context->sample_rate;
	output_codec_context->sample_fmt = output_codec->sample_fmts[0];
	output_codec_context->bit_rate = OUTPUT_BIT_RATE;

	stream->time_base.den = input_codec_context->sample_rate;
	stream->time_base.num = 1;

	if (output_format_context->oformat->flags & AVFMT_GLOBALHEADER)
		output_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (0 > avcodec_open2(output_codec_context, output_codec, nullptr))
		return false;

	if (0 > avcodec_parameters_from_context(stream->codecpar, output_codec_context))
		return false;

	return true;
}

bool AudioTranscode::init_resampler()
{
	if (0 > swr_alloc_set_opts2(&resample_context,
		&output_codec_context->ch_layout, output_codec_context->sample_fmt, output_codec_context->sample_rate,
		&input_codec_context->ch_layout, input_codec_context->sample_fmt, input_codec_context->sample_rate,
		0, nullptr)) {
		return false;
	}

	av_assert0(output_codec_context->sample_rate == input_codec_context->sample_rate);

	if (0 > swr_init(resample_context)) {
		return false;
	}

	return true;
}

bool AudioTranscode::init_fifo()
{
	fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt, output_codec_context->ch_layout.nb_channels, 1);
	if (!fifo)
		return false;

	return true;
}

bool AudioTranscode::write_output_file_header()
{
	if (0 > avformat_write_header(output_format_context, nullptr))
		return false;

	return true;
}

void AudioTranscode::OnWork()
{
	while (true)
	{
		const int output_frame_size = output_codec_context->frame_size;
		int finished = 0;
	
		while (av_audio_fifo_size(fifo) < output_frame_size)
		{
			if (readDecodeConvertAndStore(&finished))
				return;

			if (finished)
				break;
		}

		while (av_audio_fifo_size(fifo) >= output_frame_size || (finished && av_audio_fifo_size(fifo) > 0))
		{
			if (!loadEncodeAndWrite())
				return;
		}


		if (finished) {

		}
	}

	av_write_trailer(output_format_context);
}

bool AudioTranscode::readDecodeConvertAndStore(int* finished)
{
	bool bRet = false;
	int data_present;
	AVFrame* inputFrame = av_frame_alloc();
	if (!decodeAudioFrame(inputFrame, &data_present, finished))
		return false;

	uint8_t** converted_input_samples = nullptr;
	do 
	{
		if (*finished) {
			bRet = true;
			break;
		}

		if (data_present) {
			if (0 > av_samples_alloc_array_and_samples(&converted_input_samples, nullptr,
				output_codec_context->ch_layout.nb_channels, inputFrame->nb_samples,
				output_codec_context->sample_fmt, 0))
				break;

			if (0 > swr_convert(resample_context, converted_input_samples, inputFrame->nb_samples, 
				(const uint8_t**)inputFrame->extended_data, inputFrame->nb_samples))
				break;

			if (0 > av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo)+inputFrame->nb_samples))
				break;

			if (inputFrame->nb_samples > av_audio_fifo_write(fifo,(void**)converted_input_samples,inputFrame->nb_samples))
				break;

			bRet = true;
		}
	} while (false);
	
	if (converted_input_samples)
		av_freep(&converted_input_samples[0]);
	av_freep(&converted_input_samples);
	av_frame_free(&inputFrame);
	return bRet;
}

bool AudioTranscode::decodeAudioFrame(AVFrame* frame, int* data_present, int* finished)
{
	bool bRet = false;
	AVPacket* inputPacket = av_packet_alloc();
	*data_present = 0;
	*finished = 0;

	do 
	{
		int err = av_read_frame(input_format_context, inputPacket);
		if (0 > err)
		{
			if (err == AVERROR_EOF)
				*finished = true;
			else
				break;
		}

		if (0 > avcodec_send_packet(input_codec_context, inputPacket))
			break;

		err = avcodec_receive_frame(input_codec_context, frame);
		if (err == AVERROR(EAGAIN)) {
			bRet = true;
			break;
		}
		else if (err == AVERROR_EOF) {
			*finished = 1;
			bRet = true;
			break;
		}
		else if (err < 0) {
			break;
		}
		else {
			*data_present = true;
			bRet = true;
			break;
		}

	} while (false);
	
	av_packet_free(&inputPacket);
	return bRet;
}

bool AudioTranscode::loadEncodeAndWrite()
{
	const int frame_size = FFMIN(av_audio_fifo_size(fifo), output_codec_context->frame_size);

	AVFrame* output_frame = av_frame_alloc();
	output_frame->nb_samples = frame_size;
	output_frame->format = output_codec_context->sample_fmt;
	output_frame->sample_rate = output_codec_context->sample_rate;
	av_channel_layout_copy(&output_frame->ch_layout, &output_codec_context->ch_layout);

	if (0 > av_frame_get_buffer(output_frame, 0)) {
		av_frame_free(&output_frame);
		return false;
	}

	if (frame_size > av_audio_fifo_read(fifo, (void**)output_frame->data, frame_size)) {
		av_frame_free(&output_frame);
		return false;
	}

	int data_written;
	if (!encodeAudioFrame(output_frame, &data_written)) {
		av_frame_free(&output_frame);
		return false;
	}
	av_frame_free(&output_frame);
	return false;
}

bool AudioTranscode::encodeAudioFrame(AVFrame* frame, int* data_present)
{
	AVPacket* output_packet = av_packet_alloc();
	bool bRet = false;

	if (frame) {
		frame->pts = pts;
		pts += frame->nb_samples;
	}

	*data_present = 0;

	do 
	{
		int err = avcodec_send_frame(output_codec_context, frame);
		if (err < 0 && err != AVERROR_EOF) {
			break;
		}

		err = avcodec_receive_packet(output_codec_context, output_packet);
		if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
			bRet = true;
			break;
		}
		else if (err < 0) {
			break;
		}
		else {
			bRet = true;
			*data_present = true;
		}

		if (*data_present || (0 > av_write_frame(output_format_context, output_packet)))
			break;
	} while (false);
	
	av_packet_free(&output_packet);
	return bRet;
}

void AudioTranscode::clean()
{
	if (fifo)
		av_audio_fifo_free(fifo);
	swr_free(&resample_context);
	if (output_codec_context)
		avcodec_free_context(&output_codec_context);
	if (output_format_context) {
		avio_closep(&output_format_context->pb);
		avformat_free_context(output_format_context);
	}
	if (input_codec_context)
		avcodec_free_context(&input_codec_context);
	if (input_format_context)
		avformat_close_input(&input_format_context);
}
