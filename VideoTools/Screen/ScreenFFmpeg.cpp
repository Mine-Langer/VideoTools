#include "ScreenFFmpeg.h"

CScreenFFmpeg::CScreenFFmpeg()
{
	Init();
}

CScreenFFmpeg::~CScreenFFmpeg()
{

}

bool CScreenFFmpeg::GetFrame(uint8_t* buffer, int& nSize, int64_t& timestamp)
{
	bool get_state = false;
	//mCodecCtx->pix_fmt;//AV_PIX_FMT_YUVJ422P

	nSize = dstDataSize;
	timestamp = getCurTimestamp();
	int ret = av_read_frame(m_FmtCtx, m_Pkt);

	if (0 == ret) {
		//LOGI("av_read_frame mPkt->size=%d", mPkt->size);
		avcodec_send_packet(m_CodecCtx, m_Pkt);
		while (true)
		{
			ret = avcodec_receive_frame(m_CodecCtx, srcFrame);

			if (ret >= 0) 
			{
				auto img_convert_ctx = sws_getContext(srcFrame->width, srcFrame->height, m_CodecCtx->pix_fmt,
					dstFrame->width, dstFrame->height, (enum AVPixelFormat)dstFrame->format, SWS_BICUBIC, NULL, NULL, NULL);

				sws_scale(img_convert_ctx, (const unsigned char* const*)srcFrame->data,
					srcFrame->linesize, 0, srcFrame->height,
					dstFrame->data, dstFrame->linesize);
				sws_freeContext(img_convert_ctx);

				//LOGI("avcodec_receive_frame dstFrame->linesize[0]=%d", dstFrame->linesize[0]);
				memcpy(buffer, dstFrame->data[0], dstDataSize);
				get_state = true;
			}
			else 
			{
				//LOGE("ret=%d", ret);
				break;
			}

		}
		av_packet_unref(m_Pkt);
	}
	else 
	{
	}

	return get_state;
}

void CScreenFFmpeg::Init()
{
	avdevice_register_all();
	std::string name = m_Capture;
	std::string url = "video=" + name;
	const char* short_name = "dshow";

    /*
       //////gdigrab桌面截屏
       url = "desktop";
       short_name = "gdigrab";

       //////dshow桌面截屏
       url = "video=screen-capture-recorder";
       short_name = "dshow";

       //////linux桌面截屏
       url = ":1";
       short_name = "x11grab";
    */

	const AVInputFormat* inputFmt = av_find_input_format(short_name);

	AVDictionary* options = nullptr;
	//av_dict_set(&options, "list_devices", "true", 0);
	//av_dict_set(&options, "framerate", "25", 0);
	//av_dict_set(&options, "video_size", "1920x1080", 0);

#ifdef USE_DSHOW
	av_dict_set(&options, "pixel_format", "yuv420p", 0);
#endif

	if (0 != avformat_open_input(&m_FmtCtx, url.data(), inputFmt, &options)) {
		avformat_close_input(&m_FmtCtx);
		return;
	}
	if (0 != avformat_find_stream_info(m_FmtCtx, nullptr)) {
		return ;
	}
	for (int i = 0; i < m_FmtCtx->nb_streams; i++) {
		if (m_FmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_Index = i;
			m_Stream = m_FmtCtx->streams[m_Index];
			break;
		}
	}
	if (!m_Stream) {
		return;
	}

	const AVCodec* codec = avcodec_find_decoder(m_Stream->codecpar->codec_id);//AV_CODEC_ID_MJPEG
	if (!codec) {
		return;
	}
	m_CodecCtx = avcodec_alloc_context3(codec);
	if (0 != avcodec_parameters_to_context(m_CodecCtx, m_Stream->codecpar)) {
		return;
	}

	m_width = m_CodecCtx->width;
	m_height = m_CodecCtx->height;

	if (avcodec_open2(m_CodecCtx, codec, nullptr) < 0)
	{
		return;
	}
	//初始化一些读取数据时必备参数

	m_Pkt = av_packet_alloc();
	srcFrame = av_frame_alloc();

	dstFrame = av_frame_alloc();
	AVPixelFormat dstFormat = AV_PIX_FMT_RGB24;
	dstFrame->format = dstFormat;

	dstFrame->width = m_width;
	dstFrame->height = m_height;
	int dstFrame_buff_size = av_image_get_buffer_size(dstFormat, dstFrame->width, dstFrame->height, 1);
	dstFrame_buff = (uint8_t*)av_malloc(dstFrame_buff_size);
	av_image_fill_arrays(dstFrame->data, dstFrame->linesize, dstFrame_buff, dstFormat, dstFrame->width, dstFrame->height, 1);

	dstDataSize = dstFrame->width * dstFrame->height * 3;
}

void CScreenFFmpeg::Release()
{
	if (m_FmtCtx) {
		avformat_close_input(&m_FmtCtx);
		avformat_free_context(m_FmtCtx);
		m_FmtCtx = nullptr;
	}

	if (m_CodecCtx) {
		avcodec_close(m_CodecCtx);
		avcodec_free_context(&m_CodecCtx);
		m_CodecCtx = nullptr;
		m_Index = -1;
	}
	if (m_Pkt) {
		av_packet_unref(m_Pkt);
		av_packet_free(&m_Pkt);
		m_Pkt = nullptr;
	}
	if (srcFrame) {
		av_frame_free(&srcFrame);
		srcFrame = nullptr;
	}

	if (dstFrame_buff) {
		av_free(dstFrame_buff);
		dstFrame_buff = nullptr;
	}
	if (dstFrame) {
		av_frame_free(&dstFrame);
		dstFrame = nullptr;
	}
}
