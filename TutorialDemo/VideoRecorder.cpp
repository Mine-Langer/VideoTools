#include "VideoRecorder.h"
#include "Screen/CScreenDXGI.h"
#include "Screen/CScreenGDI.h"

AVFormatContext* mFmtCtx = nullptr;
AVCodecContext* mVideoCodecCtx = nullptr;
AVStream* mVideoStream = nullptr;
AVFrame* videoDstFrame = nullptr;
SwsContext* videoSwsCtx = nullptr;
AVPacket* videoPkt = nullptr;

const char* url = "tttt.mp4";
int fps = 25;
int videoBitrate = 4000000;
int width = 1920;
int height = 1080;
int mVideoIndex = -1;

AVFrame* convertRGBToAVFrame(uint8_t* rgbData, int width, int height);
AVFrame* convertRGBToYUV(AVFrame* rgbFrame, struct SwsContext* sws_ctx);

void StartEncoder()
{
	avdevice_register_all();

	const AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);

	const AVCodec* videoCodec2 = avcodec_find_encoder_by_name("libx264");

	mVideoCodecCtx = avcodec_alloc_context3(videoCodec);

	//VBR
	mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
	mVideoCodecCtx->rc_min_rate = videoBitrate / 2;
	mVideoCodecCtx->rc_max_rate = videoBitrate / 2 + videoBitrate;
	mVideoCodecCtx->bit_rate = videoBitrate;

	mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
	mVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;// AV_PIX_FMT_NV12，AV_PIX_FMT_YUV420P
	mVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	mVideoCodecCtx->time_base = { 1, fps };
	mVideoCodecCtx->framerate = { fps, 1 };
	mVideoCodecCtx->width = width;
	mVideoCodecCtx->height = width;
	mVideoCodecCtx->keyint_min = fps;
	mVideoCodecCtx->thread_count = 1;

	if (AV_CODEC_ID_H264 == mVideoCodecCtx->codec_id) 
	{
		av_opt_set(mVideoCodecCtx->priv_data, "profile", "main", 0);
		av_opt_set(mVideoCodecCtx->priv_data, "b-pyramid", "none", 0);
		av_opt_set(mVideoCodecCtx->priv_data, "preset", "superfast", 0);
		av_opt_set(mVideoCodecCtx->priv_data, "tune", "zerolatency", 0);
	}

	avcodec_open2(mVideoCodecCtx, videoCodec, nullptr);

	avformat_alloc_output_context2(&mFmtCtx, nullptr, nullptr, url);

	mVideoStream = avformat_new_stream(mFmtCtx, mVideoCodecCtx->codec);

	mVideoStream->id = mFmtCtx->nb_streams - 1;
	mVideoIndex = mVideoStream->index;
	avcodec_parameters_from_context(mVideoStream->codecpar, mVideoCodecCtx);

	mFmtCtx->video_codec_id = mFmtCtx->oformat->video_codec;
	if (mFmtCtx->oformat->flags & AVFMT_GLOBALHEADER) 
	{
		mVideoCodecCtx->flags = mVideoCodecCtx->flags | AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
		if (avio_open(&mFmtCtx->pb, url, AVIO_FLAG_WRITE) < 0) {
			return;
		}
	}

	//输出参数start
	int videoDstWidth = mVideoCodecCtx->width;
	int videoDstHeight = mVideoCodecCtx->height;
	AVPixelFormat videoDstFormat = mVideoCodecCtx->pix_fmt;//AV_PIX_FMT_YUV420P;
	videoDstFrame = av_frame_alloc();
	videoDstFrame->format = videoDstFormat;
	videoDstFrame->width = videoDstWidth;
	videoDstFrame->height = videoDstHeight;

	int videoDstFrame_buff_size = av_image_get_buffer_size(videoDstFormat, videoDstWidth, videoDstHeight, 1);
	uint8_t* videoDstFrame_buff = (uint8_t*)av_malloc(videoDstFrame_buff_size);
	av_image_fill_arrays(videoDstFrame->data, videoDstFrame->linesize, videoDstFrame_buff, videoDstFormat, videoDstWidth, videoDstHeight, 1);

	//输出参数end


	videoSwsCtx = sws_getContext(
		width, height, AV_PIX_FMT_RGB24,
		videoDstWidth, videoDstHeight, videoDstFormat,
		SWS_BILINEAR, NULL, NULL, NULL); 
	
// 	sws_getCachedContext(videoSwsCtx,
// 		width, height, AV_PIX_FMT_RGB24,
// 		videoDstWidth, videoDstHeight, videoDstFormat,
// 		0, 0, 0, 0);
	videoPkt = av_packet_alloc();
}


CVideoRecorder::CVideoRecorder()
{

}

CVideoRecorder::~CVideoRecorder()
{

}

bool CVideoRecorder::Init(int type)
{
	if (type == 1)
		m_pScreen = new CScreenDXGI();
	else if (type == 2)
		m_pScreen = new CScreenGDI();
	else if (type == 3)
	{

	}
	m_width = width;
	m_height = height;
	m_imageSize = m_width * m_height * 4;
	m_pRgbaBuffer = new uint8_t[m_imageSize]();

	return true;
}

void CVideoRecorder::Start()
{
	m_bRun = true;
	m_thread = std::thread(&CVideoRecorder::Work, this);
}

void CVideoRecorder::Stop()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_pScreen)
	{
		delete m_pScreen;
		m_pScreen = nullptr;
	}
}
void WriteFrame(AVFrame* frame);
void CVideoRecorder::Work()
{
	int ret = 0;
	int64_t timestamp = 0;

	avformat_write_header(mFmtCtx, nullptr);

	while (m_bRun)
	{
		m_pScreen->GetFrame(m_pRgbaBuffer, m_imageSize, timestamp);
		AVFrame* frame = convertRGBToAVFrame(m_pRgbaBuffer, width, height);
		AVFrame* yuvFrame = convertRGBToYUV(frame, videoSwsCtx);
		
		WriteFrame(yuvFrame);

		av_frame_free(&frame);
		av_frame_free(&yuvFrame);
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
	av_write_trailer(mFmtCtx);
}

int64_t videoPktCount = 0;
void WriteFrame(AVFrame* frame)
{
	frame->pkt_dts = frame->pts = av_rescale_q_rnd(videoPktCount, mVideoCodecCtx->time_base, mVideoStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	frame->duration = av_rescale_q_rnd(1, mVideoCodecCtx->time_base, mVideoStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

	avcodec_send_frame(mVideoCodecCtx, frame);

	while (true)
	{
		int ret = avcodec_receive_packet(mVideoCodecCtx, videoPkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			//LOGE("video avcodec_receive_packet error,EAGAIN or EOF ret=%d", ret);
			break;
		}
		else if (ret < 0) {
			break;
		}
		else
		{
			++videoPktCount;
			videoPkt->stream_index = mVideoStream->index;
			av_write_frame(mFmtCtx, videoPkt);
		}
		av_packet_unref(videoPkt);
	}
}

// 转换RGB数据到AVFrame
AVFrame* convertRGBToAVFrame(uint8_t* rgbData, int width, int height) {
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate AVFrame\n");
		return NULL;
	}

	frame->format = AV_PIX_FMT_RGB24;
	frame->width = width;
	frame->height = height;

	if (av_frame_get_buffer(frame, 32) < 0) {
		fprintf(stderr, "Could not allocate frame buffer\n");
		av_frame_free(&frame);
		return NULL;
	}

	for (int y = 0; y < height; y++) {
		memcpy(frame->data[0] + y * frame->linesize[0], rgbData + y * width * 3, width * 3);
	}

	return frame;
}

// 转换RGB AVFrame 到 YUV AVFrame
AVFrame* convertRGBToYUV(AVFrame* rgbFrame, struct SwsContext* sws_ctx) {
	AVFrame* yuvFrame = av_frame_alloc();
	if (!yuvFrame) {
		fprintf(stderr, "Could not allocate YUV frame\n");
		return NULL;
	}

	yuvFrame->format = AV_PIX_FMT_YUV420P;
	yuvFrame->width = rgbFrame->width;
	yuvFrame->height = rgbFrame->height;

	if (av_frame_get_buffer(yuvFrame, 32) < 0) {
		fprintf(stderr, "Could not allocate YUV frame buffer\n");
		av_frame_free(&yuvFrame);
		return NULL;
	}

	sws_scale(sws_ctx, rgbFrame->data, rgbFrame->linesize, 0, rgbFrame->height, yuvFrame->data, yuvFrame->linesize);

	return yuvFrame;
}