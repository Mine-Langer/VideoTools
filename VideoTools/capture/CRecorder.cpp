#include "CRecorder.h"
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

CRecorder::CRecorder()
{
	avdevice_register_all();
}

CRecorder::~CRecorder()
{

}

void CRecorder::Init(int posX, int posY, int sWidth, int sHeight)
{
	CapX = posX;
	CapY = posY;
	capWidth = sWidth;
	capHeight = sHeight;
}

bool CRecorder::Run(const char* szFile)
{
	// 设置视频参数
	if (!m_videoDecoder.OpenScreen(CapX, CapY, capWidth, capHeight))
		return false;
	
 	// 设置音频参数
	wchar_t pszDevName[MAX_PATH] = { L"audio=" };// L"audio = virtual-audio-capturer"};
	wcscat_s(pszDevName, MAX_PATH, GetMicrophoneName());
	char* szDevName = dup_wchar_to_utf8(pszDevName);
	if (!m_audioDecoder.OpenMicrophone(szDevName))
		return false;
	m_audioDecoder.SetSaveEnable(true);

	// 设置输出参数
	if (!InitOutput(szFile))
		return false;

	// 开始图像解码
	if (!m_videoDecoder.Start(this))
		return false;
	// 开始音频解码
	if (!m_audioDecoder.Start(this))
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

	Close();
}

bool CRecorder::VideoEvent(AVFrame* frame)
{
	AVFrame* vFrame = av_frame_clone(frame);
	if (vFrame == nullptr)
		return false;

	m_videoQueue.MaxSizePush(vFrame);

	return true;
}

bool CRecorder::AudioEvent(AVFrame* frame)
{
	AVFrame* aFrame = av_frame_clone(frame);
	if (aFrame == nullptr)
		return false;
	
	m_audioQueue.MaxSizePush(aFrame);
	
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
	m_thread.detach();
	return true;
}

void CRecorder::Close()
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
}


