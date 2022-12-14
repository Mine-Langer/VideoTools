#include "ConvertDemo.h"


//////////////////////////////////////////////////////////////////////////////////
bool ConvertDemo::Save(const char* szOut, const char* szInput)
{
	CDemux dm;
	dm.Open(szInput);

	return true;



	if (!OpenInput(szInput))
		return false;

	if (!OpenOutput(szOut))
		return false;

	if (!InitConfig())
		return false;

	int ret = avformat_write_header(output_fmt_ctx, nullptr);

	dowork();

	ret = av_write_trailer(output_fmt_ctx);

	Release();

	return true;
}

bool ConvertDemo::OpenInput(const char* szInput)
{
	if (0 != avformat_open_input(&input_fmt_ctx, szInput, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(input_fmt_ctx, nullptr))
		return false;

	const AVCodec* pCodec = nullptr;
	audio_index = av_find_best_stream(input_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &pCodec, 0);
	if (0 > audio_index)
		return false;

	input_codec_ctx = avcodec_alloc_context3(pCodec);
	if (!input_codec_ctx)
		return false;

	if (0 > avcodec_parameters_to_context(input_codec_ctx, input_fmt_ctx->streams[audio_index]->codecpar))
		return false;

	if (0 > avcodec_open2(input_codec_ctx, pCodec, nullptr))
		return false;

	input_codec_ctx->pkt_timebase = input_fmt_ctx->streams[audio_index]->time_base;

	return true;
}

bool ConvertDemo::OpenOutput(const char* szOutput)
{
	// 初始化输出码流的AVFormatContext
	if (0 > avformat_alloc_output_context2(&output_fmt_ctx, nullptr, nullptr, szOutput))
		return false;

	// 查找编码器
	const AVOutputFormat* out_fmt = output_fmt_ctx->oformat;
	const AVCodec* pCodec = avcodec_find_encoder(out_fmt->audio_codec);
	if (!pCodec)
		return false;

	output_codec_ctx = avcodec_alloc_context3(pCodec);
	av_channel_layout_default(&output_codec_ctx->ch_layout, 2);
	output_codec_ctx->sample_fmt = pCodec->sample_fmts[0];
	output_codec_ctx->sample_rate = 44100;
	output_codec_ctx->bit_rate = 96000;

	// 创建输出码流的AVStream
	AVStream* pStream = avformat_new_stream(output_fmt_ctx, pCodec);
	pStream->index = output_fmt_ctx->nb_streams - 1;
	pStream->time_base.den = input_codec_ctx->sample_rate;
	pStream->time_base.num = 1;

	if (out_fmt->flags & AVFMT_GLOBALHEADER)
		output_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	// 打开编码器
	if (0 > avcodec_open2(output_codec_ctx, pCodec, nullptr))
		return false;

	if (0 > avcodec_parameters_from_context(pStream->codecpar, output_codec_ctx))
		return false;

	// 打开输出文件
	if (0 > avio_open(&output_fmt_ctx->pb, szOutput, AVIO_FLAG_WRITE))
		return false;

	return true;
}


void ConvertDemo::Release()
{
	if (fifo) {
		av_audio_fifo_free(fifo);
		fifo = nullptr;
	}

	if (swr_ctx) {
		swr_free(&swr_ctx);
		swr_ctx = nullptr;
	}

	if (output_codec_ctx) {
		avcodec_close(output_codec_ctx);
		avcodec_free_context(&output_codec_ctx);
		output_codec_ctx = nullptr;
	}

	if (output_fmt_ctx) {
		avformat_close_input(&output_fmt_ctx);
		avformat_free_context(output_fmt_ctx);
		output_fmt_ctx = nullptr;
	}

	if (input_codec_ctx) {
		avcodec_close(input_codec_ctx);
		avcodec_free_context(&input_codec_ctx);
		input_codec_ctx = nullptr;
	}

	if (input_fmt_ctx) {
		avformat_close_input(&input_fmt_ctx);
		avformat_free_context(input_fmt_ctx);
		input_fmt_ctx = nullptr;
	}
}

bool ConvertDemo::InitConfig()
{
	if (0 > swr_alloc_set_opts2(&swr_ctx,
		&output_codec_ctx->ch_layout,
		output_codec_ctx->sample_fmt,
		output_codec_ctx->sample_rate,
		&input_codec_ctx->ch_layout,
		input_codec_ctx->sample_fmt,
		input_codec_ctx->sample_rate,
		0, nullptr))
		return false;

	if (0 > swr_init(swr_ctx))
		return false;

	fifo = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, output_codec_ctx->ch_layout.nb_channels, 1);
	if (!fifo)
		return false;

	input_frame = av_frame_alloc();
	input_packet = av_packet_alloc();
	output_packet = av_packet_alloc();

	return true;
}

void ConvertDemo::dowork()
{
	int err = 0;
	while (true)
	{
		err = av_read_frame(input_fmt_ctx, input_packet);
		if (err < 0)
		{
			if (err == AVERROR_EOF)
				break;
			break;
		}

		if (input_packet->stream_index == audio_index)
		{
			if (0 == avcodec_send_packet(input_codec_ctx, input_packet))
			{
				if (0 == avcodec_receive_frame(input_codec_ctx, input_frame))
				{
					PushFrameToFifo(input_frame);

					av_frame_unref(input_frame);
				}
			}

			PopFrameToEncodeAndWrite();
		}
		av_packet_unref(input_packet);
	}

	PopFrameToEncodeAndWrite();
}

void ConvertDemo::PushFrameToFifo(AVFrame* frame)
{
	AVFrame* pFrame = av_frame_alloc();
	pFrame->format = output_codec_ctx->sample_fmt;
	pFrame->ch_layout = output_codec_ctx->ch_layout;
	pFrame->nb_samples = frame->nb_samples;
	av_frame_get_buffer(pFrame, 0);

	int err = swr_convert(swr_ctx, pFrame->data, pFrame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);

	int fifo_size = av_audio_fifo_size(fifo);
	err = av_audio_fifo_realloc(fifo, fifo_size + pFrame->nb_samples);

	int wsize = av_audio_fifo_write(fifo, (void**)pFrame->data, pFrame->nb_samples);

	av_frame_free(&pFrame);
}

void ConvertDemo::PopFrameToEncodeAndWrite(bool bFinished)
{
	int err = 0;
	int fifosize = 0;
	while ((fifosize = av_audio_fifo_size(fifo)) >= output_codec_ctx->frame_size || (bFinished && fifosize > 0))
	{
		const int framesize = FFMIN(fifosize, output_codec_ctx->frame_size);

		AVFrame* outFrame = av_frame_alloc();
		outFrame->nb_samples = framesize;
		outFrame->format = output_codec_ctx->sample_fmt;
		outFrame->sample_rate = output_codec_ctx->sample_rate;
		err = av_channel_layout_copy(&outFrame->ch_layout, &output_codec_ctx->ch_layout);
		err = av_frame_get_buffer(outFrame, 0);

		err = av_audio_fifo_read(fifo, (void**)outFrame->data, framesize);

		outFrame->pts = _pts;
		_pts += outFrame->nb_samples;

		err = avcodec_send_frame(output_codec_ctx, outFrame);
		if (err == 0)
		{
			err = avcodec_receive_packet(output_codec_ctx, output_packet);
			if (err == 0)
			{
				int realtime = output_packet->pts * av_q2d(output_codec_ctx->time_base);
				printf("pts:%lld  realtime:%d  framesize:%d\r\n", output_packet->pts, realtime, input_frame->nb_samples);

				err = av_write_frame(output_fmt_ctx, output_packet);
			}
			av_packet_unref(output_packet);
		}

		av_frame_free(&outFrame);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
bool Decoder::Init(AVFormatContext* fmt_ctx, int idx)
{
	input_codec_ctx = avcodec_alloc_context3(nullptr);

	avcodec_parameters_to_context(input_codec_ctx, fmt_ctx->streams[idx]->codecpar);

	const AVCodec* pCodec = avcodec_find_decoder(input_codec_ctx->codec_id);

	avcodec_open2(input_codec_ctx, pCodec, nullptr);

	input_codec_ctx->pkt_timebase = fmt_ctx->streams[idx]->time_base;
	src_frame = av_frame_alloc();

	out_ch_layout;
	out_sample_fmt = AV_SAMPLE_FMT_FLTP;
	out_sample_rate = 44100;
	av_channel_layout_default(&out_ch_layout, 2);
	swr_alloc_set_opts2(&swr_ctx, &out_ch_layout, out_sample_fmt, out_sample_rate,
		&input_codec_ctx->ch_layout, input_codec_ctx->sample_fmt, input_codec_ctx->sample_rate, 0, nullptr);

	swr_init(swr_ctx);

	return true;
}

AVFrame* Decoder::PushPkt(AVPacket* pkt)
{
	if (0 == avcodec_send_packet(input_codec_ctx, pkt))
	{
		avcodec_receive_frame(input_codec_ctx, src_frame);

		AVFrame* cvtFrame = av_frame_alloc();
		cvtFrame->ch_layout = out_ch_layout;
		cvtFrame->sample_rate = out_sample_rate;
		cvtFrame->nb_samples = src_frame->nb_samples;
		cvtFrame->format = out_sample_fmt;
		av_frame_get_buffer(cvtFrame, 0);

		swr_convert(swr_ctx, cvtFrame->data, cvtFrame->nb_samples, (const uint8_t**)src_frame->data, src_frame->nb_samples);
		av_packet_unref(pkt);
		return cvtFrame;
	}

	return nullptr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Encoder::Init()
{
	const char* szOut = "output.mp4";
	avformat_alloc_output_context2(&output_fmt_ctx, nullptr, nullptr, szOut);

	const AVOutputFormat* out_fmt = output_fmt_ctx->oformat;

	const AVCodec* codec = avcodec_find_encoder(out_fmt->audio_codec);

	output_codec_ctx = avcodec_alloc_context3(nullptr);
	output_codec_ctx->sample_fmt = codec->sample_fmts[0];
	output_codec_ctx->sample_rate = 44100;
	output_codec_ctx->bit_rate = 96000;
	av_channel_layout_default(&output_codec_ctx->ch_layout, 2);
	output_codec_ctx->time_base = { 1, output_codec_ctx->sample_rate };

	AVStream* pStream = avformat_new_stream(output_fmt_ctx, nullptr);
	pStream->time_base = output_codec_ctx->time_base;

	if (out_fmt->flags & AVFMT_GLOBALHEADER)
		output_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	avcodec_open2(output_codec_ctx, codec, nullptr);

	avcodec_parameters_from_context(pStream->codecpar, output_codec_ctx);

	avio_open(&output_fmt_ctx->pb, szOut, AVIO_FLAG_WRITE);

	fifo = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, output_codec_ctx->ch_layout.nb_channels, 1);

	dst_pkt = av_packet_alloc();

	return true;
}

void Encoder::writeHeader()
{
	avformat_write_header(output_fmt_ctx, nullptr);
}

void Encoder::writeTail()
{
	av_write_trailer(output_fmt_ctx);
	avio_closep(&output_fmt_ctx->pb);
	avformat_close_input(&output_fmt_ctx);
	avformat_free_context(output_fmt_ctx);
}

void Encoder::writeFrame(bool bFinished)
{
	int fifosize = 0;
	while ((fifosize = av_audio_fifo_size(fifo)) >= output_codec_ctx->frame_size || (bFinished && fifosize > 0))
	{
		const int framesize = FFMIN(fifosize, output_codec_ctx->frame_size);

		AVFrame* outFrame = av_frame_alloc();
		outFrame->nb_samples = framesize;
		outFrame->format = output_codec_ctx->sample_fmt;
		outFrame->sample_rate = output_codec_ctx->sample_rate;
		outFrame->ch_layout = output_codec_ctx->ch_layout;
		av_frame_get_buffer(outFrame, 0);

		av_audio_fifo_read(fifo, (void**)outFrame->data, framesize);

		outFrame->pts = _pts;
		_pts += outFrame->nb_samples;

		if (0 == avcodec_send_frame(output_codec_ctx, outFrame))
		{
			if (avcodec_receive_packet(output_codec_ctx, dst_pkt) == 0)
			{
				//int realtime = output_packet->pts * av_q2d(output_codec_ctx->time_base);
				//printf("pts:%lld  realtime:%d  framesize:%d\r\n", output_packet->pts, realtime, input_frame->nb_samples);

				av_write_frame(output_fmt_ctx, dst_pkt);
			}
			av_packet_unref(dst_pkt);
		}

		av_frame_free(&outFrame);
	}


//	avcodec_send_frame(output_codec_ctx, frame);
//
//	avcodec_receive_packet(output_codec_ctx, dst_pkt);
//
//	av_write_frame(output_fmt_ctx, dst_pkt);
//
//	av_packet_unref(dst_pkt);
}

void Encoder::PushFrame(AVFrame* frame)
{
	int fifo_size = av_audio_fifo_size(fifo);

	av_audio_fifo_realloc(fifo, fifo_size + frame->nb_samples);

	av_audio_fifo_write(fifo, (void**)frame->data, frame->nb_samples);

	av_frame_free(&frame);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDemux::Open(const char* szinput)
{
	if (0 != avformat_open_input(&input_fmt_ctx, szinput, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(input_fmt_ctx, nullptr))
		return false;

	audio_index = av_find_best_stream(input_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (0 > audio_index)
		return false;

	decoder.Init(input_fmt_ctx, audio_index);
	
	encoder.Init();

	OnDemux();

	return true;
}

void CDemux::OnDemux()
{
	AVPacket pkt;

	encoder.writeHeader();
	while (true)
	{
		if (0 > av_read_frame(input_fmt_ctx, &pkt))
			break;
		
		if (pkt.stream_index != audio_index)
			continue;

		AVFrame* frame = decoder.PushPkt(&pkt);
		encoder.PushFrame(frame);
		
		encoder.writeFrame(false);

	}
	encoder.writeFrame(true);
	encoder.writeTail();
	printf("demux finished.\n");
}

