#include "demos.h"

/********************************************************************************/
ExtractMvs::ExtractMvs(const char* szFile)
{
	if (Open(szFile))
	{
		Run();
	}
}

ExtractMvs::~ExtractMvs()
{
	avcodec_free_context(&_video_codec_ctx);
	avformat_free_context(_format_ctx);
	av_frame_free(&_frame);
}

bool ExtractMvs::Open(const char* szFile)
{
	if (0 != avformat_open_input(&_format_ctx, szFile, nullptr, nullptr))
		return false;
	
	if (0 > avformat_find_stream_info(_format_ctx, nullptr))
		return false;

	const AVCodec* video_codec = nullptr;
	stream_video_index = av_find_best_stream(_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0);
	if (stream_video_index < 0)
		return false;

	_video_stream = _format_ctx->streams[stream_video_index];
	_video_codec_ctx = avcodec_alloc_context3(video_codec);

	avcodec_parameters_to_context(_video_codec_ctx, _video_stream->codecpar);

	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "flags2", "+export_mvs", 0);
	int ret = avcodec_open2(_video_codec_ctx, video_codec, &opts);
	av_dict_free(&opts);
	if (ret < 0)
		return false;

	av_dump_format(_format_ctx, 0, szFile, 0);

	return true;
}

void ExtractMvs::Run()
{
	_frame = av_frame_alloc();
	AVPacket pkt;
		 
	cout << "framenum, source, blockw, blockh, srcx, srcy, dstx, dsty, flags" << endl;
	
	while (true)
	{
		if (0 > av_read_frame(_format_ctx, &pkt))
			break;

		if (pkt.stream_index == stream_video_index)
			DecodePacket(&pkt);
		av_packet_unref(&pkt);
	}

	DecodePacket(nullptr);
}

bool ExtractMvs::DecodePacket(AVPacket* pkt)
{
	int ret = avcodec_send_packet(_video_codec_ctx, pkt);
	if (0 > ret)
		return false;

	while (ret >= 0)
	{
		ret = avcodec_receive_frame(_video_codec_ctx, _frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0) {
			char szError[64] = { 0 };
			av_make_error_string(szError, 64, ret);
			cout << "Error while receiving a frame from the decoder:" << szError;
			return false;
		}
		else
		{
			video_frame_count++;

			AVFrameSideData* frameSide = av_frame_get_side_data(_frame, AV_FRAME_DATA_MOTION_VECTORS);
			if (frameSide)
			{
				const AVMotionVector* mvs = (const AVMotionVector*)frameSide->data;
				for (int i = 0; i < frameSide->size/sizeof(*mvs); i++)
				{
					const AVMotionVector* mv = &mvs[i];
					printf("%d, %2d, %2d, %2d, %4d, %4d, %4d, %4d, 0x%lld\r\n",
						video_frame_count, mv->source, mv->w, mv->h,
						mv->src_x, mv->src_y, mv->dst_x, mv->dst_y, mv->flags);
				}
			}
			av_frame_unref(_frame);
		}
	}

	return true;
}

CDemos::~CDemos()
{
}

/********************************************************************************/
void CDemos::AvioReading(const char* szFile)
{
	int ret = av_file_map(szFile, &buffer, &buffer_size, 0, nullptr);
	if (ret < 0)
		return;
	
	buf_data.ptr = buffer;
	buf_data.size = buffer_size;
	
	fmt_ctx = avformat_alloc_context();
	avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, this, &read_packet, nullptr, nullptr);
	fmt_ctx->pb = avio_ctx;

	ret = avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr);
	if (ret < 0)
		return;

	ret = avformat_find_stream_info(fmt_ctx, nullptr);
	if (ret < 0)
		return;

	av_dump_format(fmt_ctx, 0, szFile, 0);

ends:
	avformat_close_input(&fmt_ctx);
	if (avio_ctx)
		av_freep(&avio_ctx->buffer);
	avio_context_free(&avio_ctx);
	
	av_file_unmap(buffer, buffer_size);
}

int CDemos::read_packet(void* opaque, uint8_t* buf, int buf_size)
{
	CDemos* pThis = (CDemos*)opaque;
	buf_size =FFMIN(buf_size, pThis->buf_data.size);
	if (!buf_size)
		return AVERROR_EOF;

	printf("ptr:%p, size:%zu\r\n", pThis->buf_data.ptr, pThis->buf_data.size);
	
	memcpy(buf, pThis->buf_data.ptr, buf_size);
	pThis->buf_data.ptr += buf_size;
	pThis->buf_data.size += buf_size;

	return buf_size;
}

/***********************************************************************************/
#define OUTPUT_CHANNEL 2
#define OUTPUT_BITRATE 96000

/***********************************************************************/
CTransAAC::~CTransAAC()
{
	Release();
}

bool CTransAAC::Run(const char* szInput, const char* szOutput)
{
	if (!OpenInput(szInput))
		return false;

	if (!OpenOutput(szOutput))
		return false;

	if (!InitConfig())
		return false;

	printf("work:\r\n");

	int ret = avformat_write_header(output_fmt_ctx, nullptr);

	dowork();

	ret = av_write_trailer(output_fmt_ctx);

	Release();

	return true;
}

bool CTransAAC::OpenInput(const char* szInput)
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

bool CTransAAC::OpenOutput(const char* szOutput)
{
	// 初始化输出码流的AVFormatContext
	if (0 > avformat_alloc_output_context2(&output_fmt_ctx, nullptr, nullptr, szOutput))
		return false;

	// 打开输出文件
	if (0 > avio_open(&output_fmt_ctx->pb, szOutput, AVIO_FLAG_WRITE))
		return false;

	// 查找编码器
	const AVOutputFormat* out_fmt = output_fmt_ctx->oformat;
	const AVCodec* pCodec = avcodec_find_encoder(out_fmt->audio_codec);
	if (!pCodec)
		return false;

	output_codec_ctx = avcodec_alloc_context3(pCodec);
	av_channel_layout_default(&output_codec_ctx->ch_layout, 2);
	//output_codec_ctx->ch_layout = input_codec_ctx->ch_layout;
	output_codec_ctx->sample_fmt = pCodec->sample_fmts[0];
	output_codec_ctx->sample_rate = input_codec_ctx->sample_rate;
	//output_codec_ctx->bit_rate = input_codec_ctx->bit_rate;
	output_codec_ctx->bit_rate = 96000;

	// 创建输出码流的AVStream
	AVStream* pStream = avformat_new_stream(output_fmt_ctx, pCodec);
	pStream->time_base.den = input_codec_ctx->sample_rate;
	pStream->time_base.num = 1;

	if (out_fmt->flags & AVFMT_GLOBALHEADER)
		output_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	// 打开编码器
	if (0 > avcodec_open2(output_codec_ctx, pCodec, nullptr))
		return false;

	if (0 > avcodec_parameters_from_context(pStream->codecpar, output_codec_ctx))
		return false;

	return true;
}

void CTransAAC::Release()
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

bool CTransAAC::InitConfig()
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

void CTransAAC::dowork()
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
					PushFrameToFifo((const uint8_t**)input_frame->extended_data, input_frame->nb_samples);

					av_frame_unref(input_frame);
				}
			}	

			PopFrameToEncodeAndWrite();
		}
		av_packet_unref(input_packet);
	}
	
	PopFrameToEncodeAndWrite();
}

void CTransAAC::PushFrameToFifo(const uint8_t** framedata, int framesize)
{
	AVFrame* pFrame = av_frame_alloc();
	pFrame->format = output_codec_ctx->sample_fmt;
	pFrame->ch_layout = output_codec_ctx->ch_layout;
	pFrame->nb_samples = framesize;
	av_frame_get_buffer(pFrame, 0);

	int err = swr_convert(swr_ctx, pFrame->data, framesize, framedata, framesize);

	int fifo_size = av_audio_fifo_size(fifo);
	err = av_audio_fifo_realloc(fifo, fifo_size+framesize);

	int wsize = av_audio_fifo_write(fifo, (void**)pFrame->data, framesize);

	av_frame_free(&pFrame);
}

void CTransAAC::PopFrameToEncodeAndWrite(bool bFinished)
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

/***********************************************************************/

CAudioConvert::~CAudioConvert()
{
	Close();
}

bool CAudioConvert::Open(const char* szfile)
{
	if (0 != avformat_open_input(&InputFormatCtx, szfile, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(InputFormatCtx, nullptr))
		return false;

	const AVCodec* pCodec = nullptr;
	if ((AudioIndex = av_find_best_stream(InputFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &pCodec, 0)) < 0)
		return false;
	
	InputCodecCtx = avcodec_alloc_context3(pCodec);
	if (!InputCodecCtx)
		return false;

	if (0 > avcodec_parameters_to_context(InputCodecCtx, InputFormatCtx->streams[AudioIndex]->codecpar))
		return false;

	if (0 > avcodec_open2(InputCodecCtx, pCodec, nullptr))
		return false;

	InputCodecCtx->pkt_timebase = InputFormatCtx->streams[AudioIndex]->time_base;

	return true;
}

bool CAudioConvert::Save(const char* szfile)
{
	if (0 > avformat_alloc_output_context2(&OutputFormatCtx, nullptr, nullptr, szfile))
		return false;

	const AVCodec* pCodec = nullptr;
	const AVOutputFormat* output_format = OutputFormatCtx->oformat;
	if (!output_format->audio_codec) 
		return false;
	
	pCodec = avcodec_find_encoder(output_format->audio_codec);
	OutputCodecCtx = avcodec_alloc_context3(pCodec);
	OutputCodecCtx->sample_fmt = pCodec->sample_fmts[0];
	OutputCodecCtx->sample_rate = InputCodecCtx->sample_rate;
	av_channel_layout_default(&OutputCodecCtx->ch_layout, OutputChannels);
	OutputCodecCtx->bit_rate = OutputBitRate;

	if (output_format->flags & AVFMT_GLOBALHEADER)
		OutputCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	AVStream* pStream = avformat_new_stream(OutputFormatCtx, pCodec);
	pStream->time_base.den = OutputCodecCtx->sample_rate;
	pStream->time_base.num = 1;

	if (0 > avcodec_parameters_from_context(pStream->codecpar, OutputCodecCtx))
		return false;

	if (0 > avcodec_open2(OutputCodecCtx, pCodec, nullptr))
		return false;

	if (0 > avio_open(&OutputFormatCtx->pb, szfile, AVIO_FLAG_WRITE))
		return false;

	if (!SetOutputOpt())
		return false;

	return true;
}

void CAudioConvert::SetOption(int nChannel, int bitRate)
{
	OutputChannels = nChannel;
	OutputBitRate = bitRate;
}

bool CAudioConvert::SetOutputOpt()
{
	if (0 > swr_alloc_set_opts2(&SwrCtx, &OutputCodecCtx->ch_layout, OutputCodecCtx->sample_fmt, OutputCodecCtx->sample_rate,
		&InputCodecCtx->ch_layout, InputCodecCtx->sample_fmt, InputCodecCtx->sample_rate, 0, nullptr))
		return false;

	if (0 > swr_init(SwrCtx))
		return false;

	Fifo = av_audio_fifo_alloc(OutputCodecCtx->sample_fmt, OutputCodecCtx->ch_layout.nb_channels, 1);
	if (!Fifo)
		return false;

	return true;
}

void CAudioConvert::Start()
{
	int err = 0;
	SrcPacket = av_packet_alloc();
	SrcFrame = av_frame_alloc();
	int nIndex = 0;

	while (true)
	{
		err = av_read_frame(InputFormatCtx, SrcPacket);
		if (err < 0)
			break;

		if (0 > avcodec_send_packet(InputCodecCtx, SrcPacket))
			continue;

		if (SrcPacket->stream_index != AudioIndex)
			continue;

		if (0 > avcodec_receive_frame(InputCodecCtx, SrcFrame))
			continue;

		uint8_t* dstBuf = nullptr;
		int dstBufSize = 0;
		int nb_samples = (int)av_rescale_rnd(
			swr_get_delay(SwrCtx, InputCodecCtx->sample_rate) + InputCodecCtx->frame_size,
			OutputCodecCtx->sample_rate, InputCodecCtx->sample_rate, AV_ROUND_INF);
		av_samples_alloc(&dstBuf, &dstBufSize, OutputCodecCtx->ch_layout.nb_channels, nb_samples, OutputCodecCtx->sample_fmt, 1);

		swr_convert(SwrCtx, &dstBuf, dstBufSize, (const uint8_t**)SrcFrame->data, SrcFrame->nb_samples);

		Sleep(1);
		printf("decode audio frame index:%d  size:%d \n", nIndex++, SrcFrame->nb_samples);
				
		av_packet_unref(SrcPacket);
		av_frame_unref(SrcFrame);
		av_free(dstBuf);
	}

	av_packet_free(&SrcPacket);
	av_frame_free(&SrcFrame);
}

void CAudioConvert::Close()
{
	if (InputFormatCtx) {
		avformat_free_context(InputFormatCtx);
	}
	if (InputCodecCtx) {
		avcodec_free_context(&InputCodecCtx);
	}
	if (OutputFormatCtx) {
		avformat_free_context(OutputFormatCtx);
	}
	if (OutputCodecCtx) {
		avcodec_free_context(&OutputCodecCtx);
	}
}

CM3u8Test::CM3u8Test()
{
	avformat_network_init();
}

CM3u8Test::~CM3u8Test()
{
	avformat_network_deinit();
}

bool CM3u8Test::OpenUrl(const char* szUrl)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szUrl, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	int AudioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	int VideoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

	return true;
}
