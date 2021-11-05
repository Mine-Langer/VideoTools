#include "audioDecoder.h"
#include <dshow.h>
#pragma comment (lib, "strmiids.lib")

namespace capture
{
	CAudioDecoder::CAudioDecoder()
	{

	}

	CAudioDecoder::~CAudioDecoder()
	{

	}

	bool CAudioDecoder::Init()
	{
		wchar_t pszDevName[MAX_PATH] = { L"audio=" };// L"audio = virtual-audio-capturer"};
		wcscat_s(pszDevName, MAX_PATH, GetMicrophoneName());
		char* szDevName = dup_wchar_to_utf8(pszDevName);

		AVInputFormat* ifmt = av_find_input_format("dshow");
		if (ifmt == nullptr)
			return false;

		if (0 != avformat_open_input(&m_pFormatCtx, szDevName, ifmt, nullptr))
			return false;

		if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
			return false;

		AVCodec* audioCodec = nullptr;
		m_audioIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &audioCodec, 0);
		if (m_audioIndex == -1)
			return false;

		m_pCodecCtx = avcodec_alloc_context3(audioCodec);
		if (m_pCodecCtx == nullptr)
			return false;

		if (0 > avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[m_audioIndex]->codecpar))
			return false;

		if (0 > avcodec_open2(m_pCodecCtx, audioCodec, nullptr))
			return false;

		return true;
	}

	bool CAudioDecoder::Start(IAudioEvent* pEvt)
	{
		m_pEvent = pEvt;
		if (!m_pEvent)
			return false;

		m_state = Started;
		m_thread = std::thread(&CAudioDecoder::OnDecodeFunction, this);

		return true;
	}

	void CAudioDecoder::OnDecodeFunction()
	{
		AVPacket packet = { 0 };
		AVFrame* srcFrame = av_frame_alloc();

		while (m_state != Stopped)
		{
			if (m_state == Paused)
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			else
			{
				if (0 > av_read_frame(m_pFormatCtx, &packet))
					continue;

				if (packet.stream_index == m_audioIndex)
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
				}
			}
		}
	}

	wchar_t* CAudioDecoder::GetMicrophoneName()
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
					SysFreeString(var.bstrVal);
				}
				pBag->Release();
			}
			pMoniker->Release();
		}
		CoUninitialize();
		return szName;
	}

};