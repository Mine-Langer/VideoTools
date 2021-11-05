#include "videoDecoder.h"

namespace capture
{
	CVideoDecoder::CVideoDecoder()
	{

	}

	CVideoDecoder::~CVideoDecoder()
	{
		Release();
	}

	bool CVideoDecoder::Init(int posX, int posY, int sWidth, int sHeight)
	{
		char szTemp[32] = { 0 };
		AVDictionary* options = nullptr;
		av_dict_set(&options, "framerate", "25", 0);
		av_dict_set(&options, "offset_x", itoa(posX, szTemp, 10), 0);
		av_dict_set(&options, "offset_y", itoa(posY, szTemp, 10), 0);
		m_imageWidth = sWidth;
		m_imageHeight = sHeight;

		AVInputFormat* ifmt = av_find_input_format("gdigrab");
		if (ifmt == nullptr)
			return false;

		if (0 != avformat_open_input(&m_pFormatCtx, "desktop", ifmt, &options))
			return false;
		
		if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
			return false;

		AVCodec* videoCodec = nullptr;
		m_videoIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &videoCodec, 0);
		if (m_videoIndex == -1)
			return false;

		if (nullptr == (m_pCodecCtx = avcodec_alloc_context3(videoCodec)))
			return false;

		if (0 > avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[m_videoIndex]->codecpar))
			return false;
		
		if (0 > avcodec_open2(m_pCodecCtx, videoCodec, nullptr))
			return false;

		m_swsCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
			m_imageWidth, m_imageHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);
		if (m_swsCtx == nullptr)
			return false;

		return true;
	}

	bool CVideoDecoder::Start(IVideoEvent* pEvt)
	{
		if (pEvt == nullptr)
			return false;

		m_pEvent = pEvt;
		m_state = Started;

		m_thread = std::thread(&CVideoDecoder::OnDecodeFunction, this);

		return true;
	}

	void CVideoDecoder::Release()
	{
		if (m_pFormatCtx)
		{
			avformat_close_input(&m_pFormatCtx);
			avformat_free_context(m_pFormatCtx);
			m_pFormatCtx = nullptr;
		}

		if (m_pCodecCtx)
		{
			avcodec_close(m_pCodecCtx);
			avcodec_free_context(&m_pCodecCtx);
			m_pCodecCtx = nullptr;
		}

		if (m_swsCtx)
		{
			sws_freeContext(m_swsCtx);
			m_swsCtx = nullptr;
		}
	}

	void CVideoDecoder::OnDecodeFunction()
	{
		AVPacket packet = { 0 };
		AVFrame* srcFrame = av_frame_alloc();

		while (m_state!=Stopped)
		{
			if (m_state == Paused)
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			else
			{
				if (0 > av_read_frame(m_pFormatCtx, &packet))
					continue;

				if (packet.stream_index == m_videoIndex)
				{
					if (0 > avcodec_send_packet(m_pCodecCtx, &packet))
					{
						av_packet_unref(&packet);
						continue;
					}

					if (0 > avcodec_receive_frame(m_pCodecCtx, srcFrame))
					{
						av_packet_unref(&packet);
						continue;
					}

					AVFrame* dstFrame = av_frame_alloc();
					dstFrame->format = AV_PIX_FMT_YUV420P;
					dstFrame->width = m_imageWidth;
					dstFrame->height = m_imageHeight;
					if (0 == av_frame_get_buffer(dstFrame, 0))
					{
						sws_scale(m_swsCtx, srcFrame->data, srcFrame->linesize, 0, m_imageHeight,
							dstFrame->data, dstFrame->linesize);

						m_pEvent->VideoEvent(dstFrame);
					}
				}
			}
		}
	}

}