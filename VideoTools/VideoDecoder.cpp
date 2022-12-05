#include "VideoDecoder.h"

CVideoDecoder::CVideoDecoder()
{

}

CVideoDecoder::~CVideoDecoder()
{

}

bool CVideoDecoder::Open(const char* szInput)
{
#if 0
	if (!m_demux.Open(szInput))
		return false;

	AVStream* pStream = m_demux.FormatContext()->streams[m_demux.VideoStreamIndex()];
	if (!pStream)
		return false;

	if (!(m_pCodecCtx = avcodec_alloc_context3(nullptr)))
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_pCodecCtx->pkt_timebase = pStream->time_base;

	m_srcWidth = m_pCodecCtx->width / 2 * 2;
	m_srcHeight = m_pCodecCtx->height / 2 * 2;
	m_srcFormat = m_pCodecCtx->pix_fmt;
#endif
	return true;
}

bool CVideoDecoder::Open(CDemultiplexer* pDemux)
{
	AVStream* pStream = pDemux->FormatContext()->streams[pDemux->VideoStreamIndex()];
	if (!pStream)
		return false;

	if (!(m_pCodecCtx = avcodec_alloc_context3(nullptr)))
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_pCodecCtx->pkt_timebase = pStream->time_base;
	m_timebase = av_q2d(pStream->time_base);

	m_srcWidth = m_pCodecCtx->width / 2 * 2;
	m_srcHeight = m_pCodecCtx->height / 2 * 2;
	m_srcFormat = m_pCodecCtx->pix_fmt;

	return true;
}

bool CVideoDecoder::OpenScreen(int posX, int posY, int sWidth, int sHeight)
{
#if 0
	char szTemp[32] = { 0 };
	AVDictionary* options = nullptr;
	av_dict_set(&options, "framerate", "25", 0);
	av_dict_set(&options, "capture_mouse_clicks", "0", 0);
	av_dict_set(&options, "offset_x", itoa(posX, szTemp, 10), 0);
	av_dict_set(&options, "offset_y", itoa(posY, szTemp, 10), 0);
	sprintf(szTemp, "%dx%d", sWidth, sHeight);
	av_dict_set(&options, "video_size", szTemp, 0);

	const AVInputFormat* ifmt = av_find_input_format("gdigrab");
	if (ifmt == nullptr)
		return false;

	if (!m_demux.Open("desktop", ifmt, &options))
		return false;

	AVStream* pStream = m_demux.FormatContext()->streams[m_demux.VideoStreamIndex()];
	if (!pStream)
		return false;

	if (!(m_pCodecCtx = avcodec_alloc_context3(nullptr)))
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_srcWidth = m_pCodecCtx->width / 2 * 2;
	m_srcHeight = m_pCodecCtx->height / 2 * 2;
	m_srcFormat = m_pCodecCtx->pix_fmt;
#endif
	return true;
}

bool CVideoDecoder::OpenCamera()
{
#if 0
	const AVInputFormat* ifmt = av_find_input_format("vfwcap");
	if (ifmt == nullptr)
		return false;

	if (!m_demux.Open("0", ifmt, nullptr))
		return false;

	AVStream* pStream = m_demux.FormatContext()->streams[m_demux.VideoStreamIndex()];
	if (!pStream)
		return false;

	if (!(m_pCodecCtx = avcodec_alloc_context3(nullptr)))
		return false;

	if (0 > avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;

	m_srcWidth = m_pCodecCtx->width / 2 * 2;
	m_srcHeight = m_pCodecCtx->height / 2 * 2;
	m_srcFormat = m_pCodecCtx->pix_fmt;
#endif
	return true;
}

bool CVideoDecoder::Start(IDecoderEvent* pEvt)
{
	if (!(m_pEvent = pEvt))
		return false;
	
	m_bRun = true;
	m_state = Started;
	m_thread = std::thread(&CVideoDecoder::OnDecodeFunction, this);

	return true;
}

void CVideoDecoder::Stop()
{
	m_state = Stopped;
	if (m_thread.joinable())
		m_thread.join();
}

bool CVideoDecoder::SendPacket(AVPacket* pkt)
{
	if (pkt == nullptr)
		m_srcVPktQueue.MaxSizePush(pkt, &m_bRun);
	else 
	{
		AVPacket* tpkt = av_packet_clone(pkt);
		if (!tpkt)
			return false;
		m_srcVPktQueue.MaxSizePush(tpkt, &m_bRun);
	}
	
	return true;
}

void CVideoDecoder::Release()
{
	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (m_pSwsCtx)
	{
		sws_freeContext(m_pSwsCtx);
		m_pSwsCtx = nullptr;
	}

	while (!m_srcVPktQueue.Empty())
	{
		AVPacket* pkt = nullptr;
		m_srcVPktQueue.Pop(pkt);
		if (pkt)
			av_packet_free(&pkt);
	}
}

bool CVideoDecoder::SetSwsConfig(SDL_Rect* rect, int width, int height, enum AVPixelFormat pix_fmt /*= AV_PIX_FMT_NONE*/)
{
	float ratio = m_srcWidth * 1.0 / (m_srcHeight * 1.0);

	m_swsFormat = (pix_fmt == AV_PIX_FMT_NONE ? AV_PIX_FMT_YUV420P : pix_fmt);

	m_swsWidth = width / 2 * 2;
	m_swsHeight = m_swsWidth * 1.0 / ratio;
	if (m_swsHeight > height)
		m_swsWidth = height * ratio;

	m_pSwsCtx = sws_getContext(m_srcWidth, m_srcHeight, m_srcFormat,
		m_swsWidth, m_swsHeight, m_swsFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);
	if (!m_pSwsCtx)
		return false;
	
	if (rect)
	{
		rect->x = (width - m_swsWidth) / 2;
		rect->y = (height - m_swsHeight) / 2;
		rect->w = m_swsWidth;
		rect->h = m_swsHeight;
	}

	return true;
}

void CVideoDecoder::GetSrcParameter(int& srcWidth, int& srcHeight, enum AVPixelFormat& srcFormat)
{
	srcWidth = m_pCodecCtx->width;
	srcHeight = m_pCodecCtx->height;
	srcFormat = m_pCodecCtx->pix_fmt;
}

void CVideoDecoder::GetSrcRational(AVRational& sampleRatio, AVRational& timebase)
{
	sampleRatio = m_pCodecCtx->sample_aspect_ratio;
	timebase = m_pCodecCtx->time_base;
}

double CVideoDecoder::Timebase()
{
	return m_timebase; 
}

AVFrame* CVideoDecoder::ConvertFrame(AVFrame* frame)
{
	AVFrame* swsFrame = av_frame_alloc();
	swsFrame->format = m_swsFormat;
	swsFrame->width = m_swsWidth;
	swsFrame->height = m_swsHeight;
	if (0 > av_frame_get_buffer(swsFrame, 0))
	{
		av_frame_free(&swsFrame);
		return nullptr;
	}

	int h = sws_scale(m_pSwsCtx, frame->data, frame->linesize, 0, m_srcHeight, swsFrame->data, swsFrame->linesize);

	swsFrame->pts = frame->pts;
	swsFrame->best_effort_timestamp = frame->pts;

	return swsFrame;
}

void CVideoDecoder::OnDecodeFunction()
{
	int error = 0;
	AVPacket* packet = nullptr;
	AVFrame* srcFrame = av_frame_alloc();

	while (m_state != Stopped && m_bRun)
	{
		if (m_state == Paused)
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		else
		{
			if (!m_srcVPktQueue.Pop(packet))
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			else
			{
				if (packet == nullptr) { // ½áÊø±êÖ¾	
					m_pEvent->VideoEvent(nullptr);
					break;
				}
				else
				{
					error = avcodec_send_packet(m_pCodecCtx, packet);
					if (error < 0)
					{
						av_packet_free(&packet);
						continue;
					}

					error = avcodec_receive_frame(m_pCodecCtx, srcFrame);
					if (error < 0)
					{
						av_packet_free(&packet);
						continue;
					}

					AVFrame* cvtFrame = ConvertFrame(srcFrame);

					m_pEvent->VideoEvent(cvtFrame);
					
					av_frame_unref(srcFrame);
					av_packet_free(&packet);
				}
			}
		}
	}
	av_frame_free(&srcFrame);

	Release();
}

/*bool CVideoDecoder::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_VIDEO)
	{
		if (pkt != nullptr)
		{
			bool bRun = (m_state != Stopped);
			AVPacket* srcVPkt = av_packet_clone(pkt);
			m_srcVPktQueue.MaxSizePush(srcVPkt, &bRun);
		}
	}
	return true;
}*/

