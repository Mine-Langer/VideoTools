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

/*
CRecorder::CRecorder()
{
	avdevice_register_all();
}

CRecorder::~CRecorder()
{

}

void CRecorder::InitVideoCfg(int posX, int posY, int sWidth, int sHeight)
{
	CapX = posX;
	CapY = posY;
	capWidth = sWidth;
	capHeight = sHeight;
}

void CRecorder::InitAudioOption(eAudioOpt audioOpt)
{
	m_audioOption = audioOpt;
}

bool CRecorder::Run(const char* szFile)
{
	// 设置视频参数
	if (!m_videoDecoder.OpenScreen(CapX, CapY, capWidth, capHeight))
		return false;

	if (!m_videoDecoder.SetSwsConfig())
		return false;

 	// 设置音频参数
	if (m_audioOption == NoAudio)
	{
		wchar_t pszDevName[MAX_PATH] = { L"audio=" };// L"audio = virtual-audio-capturer"};
		wcscat_s(pszDevName, MAX_PATH, GetMicrophoneName());
		char* szDevName = dup_wchar_to_utf8(pszDevName);

		if (!m_audioDecoder.OpenMicrophone(szDevName))
			return false;

		m_audioDecoder.SetSaveEnable(true);

		// 开始音频解码
		if (!m_audioDecoder.Start(this))
			return false;
	}

	// 设置输出参数
	if (!InitOutput(szFile))
		return false;

	// 开始图像解码
	if (!m_videoDecoder.Start(this))
		return false;

 	// 开始录制
	Start();

	return true;
}

bool CRecorder::InitOutput(const char* szOutput)
{
	m_szFilename = szOutput;
	if (0 > avformat_alloc_output_context2(&OutputFormatCtx, nullptr, nullptr, szOutput))
		return false;

	if (OutputFormatCtx->oformat->video_codec != AV_CODEC_ID_NONE)
		if (!m_videoEncoder.Init(OutputFormatCtx, OutputFormatCtx->oformat->video_codec, capWidth, capHeight))
			return false;

	if (OutputFormatCtx->oformat->audio_codec != AV_CODEC_ID_NONE)
	{
		int sample_rate, nb_sample;
		int64_t ch_layout;
		enum AVSampleFormat sample_fmt;
		m_audioDecoder.GetSrcParameter(sample_rate, nb_sample, ch_layout, sample_fmt);
		if (!m_audioEncoder.InitAudio(OutputFormatCtx, OutputFormatCtx->oformat->audio_codec, ch_layout, sample_fmt, sample_rate))
			return false;

		m_audioEncoder.BindAudioData(&m_audioQueue);
	}

	av_dump_format(OutputFormatCtx, 0, szOutput, 1);

	return true;
}

void CRecorder::Stop()
{
	m_state = Stopped;
	if (m_thread.joinable())
		m_thread.join();

	m_videoDecoder.Stop();
	m_audioDecoder.Stop();
	m_videoEncoder.Release();
	m_audioEncoder.Release();
}

bool CRecorder::VideoEvent(AVFrame* frame)
{
	AVFrame* vFrame = av_frame_clone(frame);
	if (vFrame == nullptr)
		return false;

	bool bRun = (m_state != Stopped);
	m_videoQueue.MaxSizePush(vFrame, &bRun);

	return true;
}

bool CRecorder::AudioEvent(AVFrame* frame)
{
	AVFrame* aFrame = av_frame_clone(frame);
	if (aFrame == nullptr)
		return false;
	
	bool bRun = (m_state != Stopped);
	m_audioQueue.MaxSizePush(aFrame, &bRun);
	
	return true;
}

bool CRecorder::Start()
{
	m_bRun = true;
	m_state = Started;

	if (!(OutputFormatCtx->oformat->flags & AVFMT_NOFILE))
		if (0 > avio_open(&OutputFormatCtx->pb, m_szFilename.c_str(), AVIO_FLAG_WRITE))
			return false;

	if (0 > avformat_write_header(OutputFormatCtx, nullptr))
		return false;

	// 保存数据帧线程
	m_thread = std::thread(&CRecorder::OnSaveThread, this);

	return true;
}

void CRecorder::Release()
{
	if (OutputFormatCtx) {
		avformat_close_input(&OutputFormatCtx);
		avformat_free_context(OutputFormatCtx);
		OutputFormatCtx = nullptr;
	}
}

void CRecorder::OnSaveThread()
{
	AVPacket* vPacket = nullptr;
	AVFrame* videoFrame = nullptr;
	AVPacket* aPacket = av_packet_alloc();
	av_init_packet(aPacket);
	aPacket->pts = 0;

	int videoIndex = 0;
	int audioIndex = 0;
	int ret = 0;

	while (m_state != Stopped)
	{	
		if (m_state == Paused)
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		else
		{
			if (!m_videoQueue.Pop(videoFrame))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}
			if (videoFrame == nullptr)
				break;
			// 判断音频帧/视频帧时序
			if (0 >= av_compare_ts(videoIndex, m_videoEncoder.GetTimeBase(), audioIndex, m_audioEncoder.GetTimeBase()))
			{
				// 写视频帧
				AVFrame* swsVideoFrame = m_videoDecoder.GetConvertFrame(videoFrame);
				if (!swsVideoFrame)
					continue;
				swsVideoFrame->pts = videoIndex++;

				vPacket = m_videoEncoder.Encode(swsVideoFrame);
				if (vPacket)
				{
					av_interleaved_write_frame(OutputFormatCtx, vPacket);
					av_packet_free(&vPacket);
				}

				av_frame_free(&swsVideoFrame);
				av_frame_free(&videoFrame);
			}
			else
			{
				// 写音频帧
				if (m_audioEncoder.GetEncodePacket(aPacket, audioIndex))
				{
					av_interleaved_write_frame(OutputFormatCtx, aPacket);
					av_packet_unref(aPacket);
				}
				else
				{
					av_packet_free(&aPacket);
					break;
				}
			}
		}				
	}
	av_write_trailer(OutputFormatCtx);

	Release();
}

wchar_t* CRecorder::GetMicrophoneName()
{
	CoInitialize(nullptr);
	static wchar_t szName[MAX_PATH] = { 0 };
	ICreateDevEnum* pCreateDevEnum = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (LPVOID*)&pCreateDevEnum);
	if (FAILED(hr))
		return szName;

	IEnumMoniker* pEnumMoniker = nullptr;
	if (FAILED(pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumMoniker, 0)))
	{
		CoUninitialize();
		return szName;
	}

	pEnumMoniker->Reset();
	
	ULONG ulFetch = 0;
	IMoniker* pMoniker = nullptr;
	while (S_OK == (pEnumMoniker->Next(1, &pMoniker, &ulFetch)))
	{
		IPropertyBag* pBag = nullptr;
		if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag)))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			if (SUCCEEDED(pBag->Read(L"FriendlyName", &var, nullptr)))
			{
				wcscpy_s(szName, MAX_PATH, var.bstrVal);
				//WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, szName, MAX_PATH, "", nullptr);
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pMoniker->Release();
	}
	CoUninitialize();
	return szName;
}*/

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

	AVOutputFormat* pOutputFmt = m_pFormatCtx->oformat;
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
// 				if (pvFrame == nullptr)
// 					break;
				// AVFrame* swsFrame = m_videoDecoder.ConvertFrame(pvFrame);
				m_captureScreen.CaptureImage();
				{
					m_swsFrame->pkt_dts = m_swsFrame->pts = videoIndex++;
					//m_swsFrame->pkt_duration = 90000 / 25 / 100;
					//m_swsFrame->pkt_pos = -1;

					AVPacket* pkt = m_videoEncoder.Encode(m_swsFrame);
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
					}
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
