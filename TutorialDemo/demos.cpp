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
	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &buf_data, &read_packet, nullptr, nullptr);
	fmt_ctx->pb = avio_ctx;
}

int CDemos::read_packet(void* opaque, uint8_t* buf, int buf_size)
{
	return 0;
}
