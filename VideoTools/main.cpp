#include "Common.h"
#include "AVMainWnd.h"
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QTextStream>

#if 0
typedef struct {
	uint8_t* buffer;
	size_t buffer_size;
	size_t current_pos;
} BufferData;

// 读取回调函数
static int read_packet(void* opaque, uint8_t* buf, int buf_size) {
	BufferData* bd = (BufferData*)opaque;
	buf_size = FFMIN(buf_size, bd->buffer_size - bd->current_pos);
	if (!buf_size)
		return AVERROR_EOF;
	memcpy(buf, bd->buffer + bd->current_pos, buf_size);
	bd->current_pos += buf_size;
	return buf_size;
}

// 关闭回调函数
static int64_t seek_packet(void* opaque, int64_t offset, int whence) {
	BufferData* bd = (BufferData*)opaque;
	if (whence == AVSEEK_SIZE)
		return bd->buffer_size;
	if (whence == SEEK_SET)
		bd->current_pos = offset;
	else if (whence == SEEK_CUR)
		bd->current_pos += offset;
	else if (whence == SEEK_END)
		bd->current_pos = bd->buffer_size + offset;
	return bd->current_pos;
}

AVFormatContext* open_input_from_memory(uint8_t* buffer, size_t buffer_size) {
	AVFormatContext* fmt_ctx = NULL;
	AVIOContext* avio_ctx = NULL;
	uint8_t* avio_ctx_buffer = NULL;
	size_t avio_ctx_buffer_size = 4096;
	BufferData bd = { buffer, buffer_size, 0 };

	fmt_ctx = avformat_alloc_context();
	if (!fmt_ctx) {
		return nullptr;
	}

	avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
	if (!avio_ctx_buffer) {
		avformat_free_context(fmt_ctx);
		return nullptr;
	}

	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &bd, read_packet, NULL, seek_packet);
	if (!avio_ctx) {
		av_free(avio_ctx_buffer);
		avformat_free_context(fmt_ctx);
		return nullptr;
	}

	fmt_ctx->pb = avio_ctx;
	fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

	int ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
	if (ret < 0) {
		avio_context_free(&avio_ctx);
		avformat_free_context(fmt_ctx);
		return nullptr;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		avformat_close_input(&fmt_ctx);
		avio_context_free(&avio_ctx);
		return nullptr;
	}

	return fmt_ctx;
}

// 将图片加载为AVFrame
AVFrame* load_image(const char* filename/*, AVPixelFormat pix_fmt, int width, int height*/)
{
	AVFormatContext* fmt_ctx = NULL;
	AVCodecContext* codec_ctx = NULL;
	const AVCodec* codec = NULL;
	AVFrame* frame = NULL;
	AVPacket packet;

	if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open input file %s\n", filename);
		return NULL;
	}

	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information\n");
		return NULL;
	}

	int video_stream_index = -1;
	for (int i = 0; i < fmt_ctx->nb_streams; i++) {
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			break;
		}
	}
	if (video_stream_index == -1) {
		fprintf(stderr, "Could not find video stream\n");
		return NULL;
	}

	codec = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "Could not find codec\n");
		return NULL;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		fprintf(stderr, "Could not allocate codec context\n");
		return NULL;
	}

	if (avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar) < 0) {
		fprintf(stderr, "Could not copy codec parameters to context\n");
		return NULL;
	}

	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return NULL;
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate frame\n");
		return NULL;
	}

	while (av_read_frame(fmt_ctx, &packet) >= 0) {
		if (packet.stream_index == video_stream_index) {
			if (avcodec_send_packet(codec_ctx, &packet) == 0) {
				if (avcodec_receive_frame(codec_ctx, frame) == 0) {
					break;
				}
			}
		}
		av_packet_unref(&packet);
	}

	av_packet_unref(&packet);
	avcodec_free_context(&codec_ctx);
	avformat_close_input(&fmt_ctx);

	return frame;
}

// 将图片加载为AVFrame
AVFrame* load_image2(const char* filename/*, AVPixelFormat pix_fmt, int width, int height*/)
{
	AVCodecContext* codec_ctx = NULL;
	const AVCodec* codec = NULL;
	AVFrame* frame = NULL;
	AVPacket packet;

	// 打开 Qt 资源文件
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		return nullptr;
	}

	QByteArray fileData = file.readAll();
	file.close();

	AVFormatContext* fmt_ctx = open_input_from_memory((uint8_t*)fileData.data(), fileData.size());
	if (!fmt_ctx) {
		return nullptr;
	}

	int video_stream_index = -1;
	for (int i = 0; i < fmt_ctx->nb_streams; i++) {
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			break;
		}
	}
	if (video_stream_index == -1) {
		fprintf(stderr, "Could not find video stream\n");
		return NULL;
	}

	codec = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "Could not find codec\n");
		return NULL;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		fprintf(stderr, "Could not allocate codec context\n");
		return NULL;
	}

	if (avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar) < 0) {
		fprintf(stderr, "Could not copy codec parameters to context\n");
		return NULL;
	}

	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return NULL;
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate frame\n");
		return NULL;
	}

	while (av_read_frame(fmt_ctx, &packet) >= 0) {
		if (packet.stream_index == video_stream_index) {
			if (avcodec_send_packet(codec_ctx, &packet) == 0) {
				if (avcodec_receive_frame(codec_ctx, frame) == 0) {
					break;
				}
			}
		}
		av_packet_unref(&packet);
	}

	av_packet_unref(&packet);
	avcodec_free_context(&codec_ctx);
	avformat_close_input(&fmt_ctx);

	return frame;
}

// 将两个AVFrame合成
void add_watermark(AVFrame* frame, AVFrame* watermark_frame, AVFilterGraph* filter_graph, AVFilterContext* buffersink_ctx, AVFilterContext* buffersrc_ctx, AVFilterContext* watermark_src_ctx)
{
	if (av_buffersrc_add_frame(buffersrc_ctx, frame) < 0) {
		fprintf(stderr, "Error while feeding the filtergraph\n");
		return;
	}

	if (av_buffersrc_add_frame_flags(watermark_src_ctx, watermark_frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
		fprintf(stderr, "Error while feeding the filtergraph with watermark\n");
		return;
	}

	while (1) {
		AVFrame* filt_frame = av_frame_alloc();
		int ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_frame_free(&filt_frame);
			break;
		}
		if (ret < 0) {
			av_frame_free(&filt_frame);
			break;
		}

		// 处理合成后的帧（例如，编码、保存或显示）
		// 此处仅释放帧
		av_frame_free(&filt_frame);
	}
}

AVFrame* QImageToAVFrame(const QImage& img) {
	AVPixelFormat pix_fmt;
	switch (img.format()) {
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32:
	case QImage::Format_ARGB32_Premultiplied:
		pix_fmt = AV_PIX_FMT_ARGB;
		break;
	case QImage::Format_RGB888:
		pix_fmt = AV_PIX_FMT_RGB24;
		break;
	case QImage::Format_RGB16:
		pix_fmt = AV_PIX_FMT_RGB565;
		break;
	case QImage::Format_Indexed8:
	case QImage::Format_Grayscale8:
		pix_fmt = AV_PIX_FMT_GRAY8;
		break;
	default:
		return nullptr;
	}

	// 创建 AVFrame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		return nullptr;
	}

	// 设置 AVFrame 的属性
	frame->format = AV_PIX_FMT_RGBA;
	frame->width = img.width();
	frame->height = img.height();

	// 为 AVFrame 分配缓冲区
	int ret = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, pix_fmt, 32);
	if (ret < 0) {
		av_frame_free(&frame);
		return nullptr;
	}

	// 将 QImage 的数据复制到 AVFrame
	for (int y = 0; y < img.height(); ++y) {
		memcpy(frame->data[0] + y * frame->linesize[0], img.scanLine(y), img.bytesPerLine());
	}

	return frame;
}

int test_filter()
{
	const char* input_filename = "8o0c3k.png";
	const char* watermark_filename = "watermask.png";

	// 加载输入图片和水印图片
	AVFrame* frame = load_image(input_filename);
	if (!frame) {
		fprintf(stderr, "Could not load input image\n");
		return -1;
	}

	AVFrame* watermark_frame2 = load_image(watermark_filename);
	AVFrame* watermark_frame = load_image2(":/HLCapture/res/watermask.png");
	AVFrame* watermark_frame3 = QImageToAVFrame(QImage("watermask.png")); // load_image(watermark_filename);
	if (!watermark_frame) {
		fprintf(stderr, "Could not load watermark image\n");
		return -1;
	}

	// 设置滤镜图形
	AVFilterGraph* filter_graph = avfilter_graph_alloc();
	if (!filter_graph) {
		fprintf(stderr, "Could not allocate filter graph\n");
		return -1;
	}

	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	const AVFilter* overlay = avfilter_get_by_name("overlay");

	AVFilterContext* buffersrc_ctx = avfilter_graph_alloc_filter(filter_graph, buffersrc, "in");
	AVFilterContext* watermark_src_ctx = avfilter_graph_alloc_filter(filter_graph, buffersrc, "watermark");
	AVFilterContext* buffersink_ctx = avfilter_graph_alloc_filter(filter_graph, buffersink, "out");

	char args[512];
	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		frame->width, frame->height, frame->format,
		1, 25, 1, 1);
	avfilter_init_str(buffersrc_ctx, args);

	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		watermark_frame->width, watermark_frame->height, watermark_frame->format,
		1, 25, 1, 1);
	avfilter_init_str(watermark_src_ctx, args);

	avfilter_init_str(buffersink_ctx, NULL);

	AVFilterContext* overlay_ctx = avfilter_graph_alloc_filter(filter_graph, overlay, "overlay");
	avfilter_init_str(overlay_ctx, "10:10");

	avfilter_link(buffersrc_ctx, 0, overlay_ctx, 0);
	avfilter_link(watermark_src_ctx, 0, overlay_ctx, 1);
	avfilter_link(overlay_ctx, 0, buffersink_ctx, 0);

	if (avfilter_graph_config(filter_graph, NULL) < 0) {
		fprintf(stderr, "Could not configure filter graph\n");
		return -1;
	}

	// 合成水印
	add_watermark(frame, watermark_frame, filter_graph, buffersink_ctx, buffersrc_ctx, watermark_src_ctx);

	av_frame_free(&frame);
	av_frame_free(&watermark_frame);
	avfilter_graph_free(&filter_graph);

	return 0;
}
#endif

//https://mnkwxsheem9.haifengsports.club/20221018/rt0EzjWF/index.m3u8
#undef main
int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    
	// test_filter();

    QFile qssFile(":/AvTools/res/style.qss");
    if (qssFile.exists())
    {
        qssFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&qssFile);
        in.setCodec("UTF-8");
        QString qss = in.readAll();
        qApp->setStyleSheet(qss);
        qssFile.close();
    }

    AVMainWnd w;
    w.show();

    return a.exec();
}
