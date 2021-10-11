#include "CVideoEncoder.h"

CVideoEncoder::CVideoEncoder()
{

}

CVideoEncoder::~CVideoEncoder()
{

}

bool CVideoEncoder::InitConfig(AVFormatContext* outputFmtCtx, int width, int height)
{
	VideoStream = avformat_new_stream(outputFmtCtx, nullptr);
	if (VideoStream == nullptr)
		return false;
	VideoStream->time_base = { 1, 25 };
	VideoStream->id = outputFmtCtx->nb_streams - 1;

	AVCodec* codec = avcodec_find_encoder(outputFmtCtx->oformat->video_codec);
	if (codec == nullptr)
		return true;

	VideoCodecCtx = avcodec_alloc_context3(codec);
	if (VideoCodecCtx == nullptr)
		return false;

	VideoCodecCtx->codec_id = outputFmtCtx->oformat->video_codec;
	VideoCodecCtx->bit_rate = 400000;
	VideoCodecCtx->width = width;
	VideoCodecCtx->height = height;
	VideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	VideoCodecCtx->time_base = VideoStream->time_base;
	VideoCodecCtx->gop_size = 12;

	if (outputFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
		VideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	AVDictionary* options = nullptr;
	if (VideoCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		av_dict_set(&options, "preset", "slow", 0);
		av_dict_set(&options, "tune", "zerolatency", 0);
	}

	if (0 > avcodec_open2(VideoCodecCtx, codec, &options))
		return false;

	avcodec_parameters_from_context(VideoStream->codecpar, VideoCodecCtx);

	return true;
}

void CVideoEncoder::Start(IEncoderEvent* evt)
{
	event = evt;

}

void CVideoEncoder::Encode(AVFrame* frame)
{
	if (0 > avcodec_send_frame(VideoCodecCtx, frame))
		return;

	AVPacket pkt = { 0 };
	int ret = avcodec_receive_packet(VideoCodecCtx, &pkt);
	if (ret == 0)
	{
		av_packet_rescale_ts(&pkt, VideoCodecCtx->time_base, VideoStream->time_base);
		pkt.stream_index = VideoStream->index;

		event->VideoEvent(&pkt);
	}
}
