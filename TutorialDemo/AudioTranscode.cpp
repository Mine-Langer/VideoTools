#include "AudioTranscode.h"

#define OUTPUT_BIT_RATE 96000
#define OUTPUT_CHANNELS 2

void AudioTranscode::Run()
{
	if (!open_input_file("output.aac")) // "F:/CloudMusic/Paradox Interactive - The Titan.mp3"
		return;

	if (!open_output_file("output.mp3"))
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
	if (0 > avformat_open_input(&input_format_context, filename, nullptr, nullptr)) {
		return false;
	}

	if (0 > avformat_find_stream_info(input_format_context, nullptr)) {
		return false;
	}

	const AVCodec* input_codec = nullptr;
	audio_index = av_find_best_stream(input_format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &input_codec, 0);
	if (0 > audio_index)
		return false;

	const AVStream* stream = input_format_context->streams[audio_index];

	input_codec_context = avcodec_alloc_context3(input_codec);
	if (!input_codec_context) {
		return false;
	}

	if (0 > avcodec_parameters_to_context(input_codec_context, stream->codecpar)) {
		return false;
	}

	if (0 > avcodec_open2(input_codec_context, input_codec, NULL)) {
		return false;
	}

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
	int ret = 0;
	AVFrame* input_frame = av_frame_alloc();
	input_packet = av_packet_alloc();

	while (true)
	{
		ret = decodeAudioFrame(input_frame);
		if (ret == 1)
		{
			ret = convertStore(input_frame);
			if (ret == 1)
				loadEncodeAndWrite();
			//encodeAudioFrame(input_frame);
		}
		else if (ret == 2){
			break;
		}
	}

	av_frame_free(&input_frame);
	av_packet_free(&input_packet);

	av_write_trailer(output_format_context);
}

int AudioTranscode::decodeAudioFrame(AVFrame* frame)
{
	int err = av_read_frame(input_format_context, input_packet);
	if (err < 0) {
		if (err == AVERROR_EOF) {
			return 2;
		}
		return 0;
	}
	
	if (input_packet->stream_index != audio_index) {
		av_packet_unref(input_packet);
		return 0;
	}

	err = avcodec_send_packet(input_codec_context, input_packet);
	if (err < 0) {
		av_packet_unref(input_packet);
		return 0;
	}

	av_packet_unref(input_packet);
	err = avcodec_receive_frame(input_codec_context, frame);
	if (err < 0)
		return 0;

	return 1;
}

int AudioTranscode::convertStore(AVFrame* frame)
{
	AVFrame* cvt_frame = av_frame_alloc();
	cvt_frame->nb_samples = frame->nb_samples;
	cvt_frame->format = output_codec_context->sample_fmt;
	cvt_frame->sample_rate = output_codec_context->sample_rate;
	av_channel_layout_copy(&cvt_frame->ch_layout, &output_codec_context->ch_layout);
	if (0 > av_frame_get_buffer(cvt_frame, 0)) {
		av_frame_free(&cvt_frame);
		return 0;
	}

	if (0 > swr_convert(resample_context, cvt_frame->data, cvt_frame->nb_samples, 
		(const uint8_t**)frame->data, frame->nb_samples)) { 
		av_frame_free(&cvt_frame);
		return 0;
	}

	cvt_frame->pts = frame->pts;
	cvt_frame->best_effort_timestamp = frame->best_effort_timestamp;

	int frame_size = cvt_frame->nb_samples;
	int fifo_size = av_audio_fifo_size(fifo);
	av_audio_fifo_realloc(fifo, frame_size + fifo_size);
	av_audio_fifo_write(fifo, (void**)cvt_frame->data, frame_size);

	av_frame_free(&cvt_frame);

	return 1;
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

	if (!encodeAudioFrame(output_frame)) {
		av_frame_free(&output_frame);
		return false;
	}
	av_frame_free(&output_frame);
	return false;
}

int AudioTranscode::encodeAudioFrame(AVFrame* frame)
{
	AVPacket* output_packet = av_packet_alloc();

	if (frame) {
		frame->pts = pts;
		pts += frame->nb_samples;
	}

	int err = avcodec_send_frame(output_codec_context, frame);
	if (err < 0 && err != AVERROR_EOF) {
		av_packet_free(&output_packet);
		return 0;
	}

	err = avcodec_receive_packet(output_codec_context, output_packet);
	if (err < 0) {
		av_packet_free(&output_packet);
		return 0;
	}
	else if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
		av_packet_free(&output_packet);
		return 1;
	}
	else {
		av_write_frame(output_format_context, output_packet);
	}
	
	av_packet_free(&output_packet);
	return 1;
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
