#include "CompositeVideo.h"

CompositeVideo::CompositeVideo()
{

}

CompositeVideo::~CompositeVideo()
{

}

bool CompositeVideo::Open(const char* szFile)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szFile, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	AVCodec* videoCodec = nullptr;
	m_videoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &videoCodec, 0);
	if (m_videoIndex == -1)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(videoCodec);
	if (m_pCodecCtx == nullptr)
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[m_videoIndex]->codecpar))
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, videoCodec, nullptr))
		return false;

	m_filter.SetFilter("drawtext=fontfile=arial.ttf:x=100:y=100:fontcolor=red:fontsize=48:text='Fuck CCP!'");
	m_filter.Init(m_pCodecCtx, m_pFormatCtx->streams[m_videoIndex]);

	return true;
}

void CompositeVideo::Start(IVideoEvent* pEvt)
{
	m_pEvent = pEvt;
	m_thread = std::thread(&CompositeVideo::OnDecodeFunction, this);
}

void CompositeVideo::Release()
{
	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

bool CompositeVideo::SetFrameSize(int width, int height, enum AVPixelFormat pix_fmt)
{
	m_swsWidth = width;
	m_swsHeight = height;
	m_swsPixfmt = pix_fmt;
	if (m_pSwsCtx) {
		sws_freeContext(m_pSwsCtx);
		m_pSwsCtx = nullptr;
	}
	m_pSwsCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
		m_swsWidth, m_swsHeight, pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
	if (m_pSwsCtx == nullptr)
		return false;

	return true;
}

AVFrame* CompositeVideo::ConvertFrame(AVFrame* frame)
{
	AVFrame* swsFrame = av_frame_alloc();
	swsFrame->format = m_swsPixfmt;
	swsFrame->width = m_swsWidth;
	swsFrame->height = m_swsHeight;
	if (0 > av_frame_get_buffer(swsFrame, 0))
		return nullptr;

	sws_scale(m_pSwsCtx, frame->data, frame->linesize, 0, m_pCodecCtx->height, swsFrame->data, swsFrame->linesize);

	AVFrame* dstFrame = m_filter.Convert(swsFrame);
	av_frame_free(&swsFrame);

	return dstFrame;
}

int CompositeVideo::GetSrcWidth()
{
	return m_pCodecCtx->width / 2 * 2;
}

int CompositeVideo::GetSrcHeight()
{
	return m_pCodecCtx->height / 2 * 2;
}

AVPixelFormat CompositeVideo::GetSrcFormat()
{
	return m_pCodecCtx->pix_fmt;
}

void CompositeVideo::OnDecodeFunction()
{
	int ret = 0;
	AVPacket packet = { 0 };
	AVFrame* frame = av_frame_alloc();

	while (true)
	{
		if (0 > av_read_frame(m_pFormatCtx, &packet))
			break;

		if (packet.stream_index == m_videoIndex)
		{
			if (0 > avcodec_send_packet(m_pCodecCtx, &packet))
				break;

			ret = avcodec_receive_frame(m_pCodecCtx, frame);
			if (ret < 0)
			{
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					av_frame_unref(frame);
					av_packet_unref(&packet);
					continue;
				}
				break;
			}
			else if (ret == 0)
			{
				m_pEvent->VideoEvent(frame);
			}
			av_frame_unref(frame);
		}
		av_packet_unref(&packet);
	}
	av_frame_free(&frame);
}
