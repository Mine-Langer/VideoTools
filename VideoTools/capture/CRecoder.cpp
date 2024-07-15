#include "CRecoder.h"
#include <dshow.h>
#pragma comment (lib, "strmiids.lib")

char* dup_wchar_to_utf8(const wchar_t* w)
{
	char* s = NULL;
	int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	s = (char*)av_malloc(l);
	if (s)
		WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
	return s;
}


CRecoder::CRecoder()
{
	avdevice_register_all();
}

CRecoder::~CRecoder()
{

}

void CRecoder::SetVideoOption(int x, int y, int w, int h)
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
}

void CRecoder::SetAudioOption(enum eAudioOpt audioOpt)
{
	m_audioOption = audioOpt;
}

void CRecoder::SetSaveFile(const char* szName)
{
	m_szFile = szName;
}

bool CRecoder::Start()
{
	if (!InitInput())
		return false;

	if (!InitOutput())
		return false;

	m_status = Started;
	m_thread = std::thread(&CRecoder::CaptureThread, this);

	return true;
}

void CRecoder::Pause()
{
	m_status = Paused;
}

void CRecoder::Stop()
{
	m_status = Stopped;
	if (m_thread.joinable())
		m_thread.join();
	
	Close();
}

void CRecoder::Close()
{
//	m_videoDecoder.Stop();

	av_frame_free(&m_swsFrame);

	m_videoEncoder.Release();

	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}
}

bool CRecoder::InitOutput()
{
	if (0 > avformat_alloc_output_context2(&m_pFormatCtx, nullptr, nullptr, m_szFile.c_str()))
		return false;

	const AVOutputFormat* pOutputFmt = m_pFormatCtx->oformat;
	if (pOutputFmt->video_codec != AV_CODEC_ID_NONE)
	{
		if (!m_videoEncoder.Init(m_pFormatCtx, pOutputFmt->video_codec, m_w, m_h))
			return false;
	}

	if (pOutputFmt->audio_codec != AV_CODEC_ID_NONE)
	{

	}

	av_dump_format(m_pFormatCtx, 0, m_szFile.c_str(), 1);
	
	return true;
}

bool CRecoder::InitInput()
{
// 	if (!m_videoDecoder.OpenScreen(m_x, m_y, m_w, m_h))
// 		return false;
// 	if (!m_videoDecoder.SetSwsConfig())
// 		return false;
// 	if (!m_videoDecoder.Start(this))
// 		return false;
	m_captureScreen.Init(m_x, m_y, m_w, m_h);

	m_swsFrame = av_frame_alloc();
	if (!m_swsFrame)
		return false;

	m_swsFrame->format = AV_PIX_FMT_YUV420P;
	m_swsFrame->width = m_w;
	m_swsFrame->height = m_h;
	int64_t frameSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, m_w, m_h, 1);
	av_image_fill_arrays(m_swsFrame->data, m_swsFrame->linesize, m_captureScreen.YUVBuffer(), AV_PIX_FMT_YUV420P, m_w, m_h, 1);

	return true;
}

void CRecoder::CaptureThread()
{
	AVFrame* pvFrame = nullptr;
	unsigned int videoIndex = 0;
	uint64_t lastPts = 0, lastDts = 0;

	if (!WriteHeader())
		return;

	while (m_status!=Stopped)
	{
		if (m_status == Paused)
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			{
				m_captureScreen.CaptureImage();
				{
					m_swsFrame->pkt_dts = m_swsFrame->pts = videoIndex++;
					//m_swsFrame->pkt_duration = 90000 / 25 / 100;
					//m_swsFrame->pkt_pos = -1;

					/*AVPacket* pkt = m_videoEncoder.Encode(m_swsFrame);
					if (pkt)
					{
						pkt->duration = 1024;
						pkt->pts = lastPts + pkt->duration;
						pkt->dts = lastDts + pkt->duration;
						lastPts = pkt->pts;
						lastDts = pkt->dts;
// 						pkt->pts = -1;

						av_write_frame(m_pFormatCtx, pkt);
						//av_interleaved_write_frame(m_pFormatCtx, pkt);
						av_packet_free(&pkt);
					}*/
				}

				av_frame_free(&pvFrame);
			}
		}
	}

	WriteTrailer();

	Close();
}

bool CRecoder::WriteHeader()
{
	if (!(m_pFormatCtx->oformat->flags & AVFMT_NOFILE))
		if (0 > avio_open(&m_pFormatCtx->pb, m_szFile.c_str(), AVIO_FLAG_WRITE))
			return false;

	if (0 > avformat_write_header(m_pFormatCtx, nullptr))
		return false;

	return true;
}

void CRecoder::WriteTrailer()
{
	av_write_trailer(m_pFormatCtx);
}

bool CRecoder::VideoEvent(AVFrame* frame)
{
	bool bRun = (m_status != Stopped);
	if (frame)
	{
		AVFrame* vframe = av_frame_clone(frame);
		m_vDataQueue.MaxSizePush(vframe, &bRun);
		return true;
	}
	m_vDataQueue.MaxSizePush(frame, &bRun);
	return false;
}

bool CRecoder::AudioEvent(AVFrame* frame)
{
	if (frame)
	{
		return true;
	}
	return false;
}
