#include "CapAudio.h"
#include <dshow.h>
#pragma comment (lib, "strmiids.lib")

CCapAudio::CCapAudio()
{

}

CCapAudio::~CCapAudio()
{

}

bool CCapAudio::Init(enum AVSampleFormat sample_fmt, int nChannels, int channel_layout, int sample_rate, int frame_size)
{
	wchar_t pszDevName[MAX_PATH] = { L"audio=" };// L"audio = virtual-audio-capturer"};
	wcscat_s(pszDevName, MAX_PATH, GetMicrophoneName());
	char* szDevName = dup_wchar_to_utf8(pszDevName);
	m_frameSize = frame_size;
	m_outSampleRate = sample_rate;
	m_outChannels = nChannels;
	m_outChannelLayout = channel_layout;
	m_outSampleFmt = sample_fmt;

	const AVInputFormat* ifmt = av_find_input_format("dshow");
	if (ifmt == nullptr)
		return false;

	if (0 != avformat_open_input(&m_pFormatCtx, szDevName, ifmt, nullptr))
		return false;

	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;

	const AVCodec* audioCodec = nullptr;
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

	m_swrCtx = swr_alloc_set_opts(nullptr, channel_layout, sample_fmt, sample_rate,
		m_pCodecCtx->channel_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr);
	if (m_swrCtx == nullptr)
		return false;
	av_opt_set_int(m_swrCtx, "in_channel_count", m_pCodecCtx->channels, 0);
	av_opt_set_int(m_swrCtx, "out_channel_count", nChannels, 0);

	if (0 > swr_init(m_swrCtx))
		return false;

	m_audioFifo = av_audio_fifo_alloc(sample_fmt, nChannels, 1);
	if (m_audioFifo == nullptr)
		return false;

	return true;
}

bool CCapAudio::Start(IDecoderEvent* pEvt)
{
	m_pEvent = pEvt;
	if (!m_pEvent)
		return false;

	m_state = Started;
	m_thread = std::thread(&CCapAudio::OnDecodeFunction, this);
	m_tConvert = std::thread(&CCapAudio::OnConvertFunction, this);

	return true;
}

void CCapAudio::Release()
{

}

void CCapAudio::OnDecodeFunction()
{
	AVPacket packet = { 0 };
	AVFrame* srcFrame = av_frame_alloc();
	AVFrame* dstFrame = av_frame_alloc();

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

				PushFrame(srcFrame);

				/*int dstNbSamples = av_rescale_rnd(swr_get_delay(m_swrCtx, m_pCodecCtx->sample_rate)+srcFrame->nb_samples,
					m_outSampleRate, m_pCodecCtx->sample_rate, AV_ROUND_UP);
			
				if (0 > av_samples_alloc(dstFrame->data, dstFrame->linesize, m_outChannels, dstNbSamples, m_outSampleFmt, 1))
					return;

				if (0 > swr_convert(m_swrCtx, dstFrame->data, dstNbSamples, (const uint8_t**)srcFrame->data, srcFrame->nb_samples))
					return;

				if (srcFrame->nb_samples > av_audio_fifo_write(m_audioFifo, (void**)dstFrame->data, dstFrame->nb_samples))
					return;*/

				av_frame_unref(srcFrame);
			}
		}
	}
}

void CCapAudio::OnConvertFunction()
{
	while (m_state != Stopped)
	{
		if (m_audioQueue.empty())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		else
		{
			AVFrame* dstFrame = av_frame_alloc();
			while (av_audio_fifo_size(m_audioFifo) < m_frameSize)
			{
				AVFrame* aFrame = m_audioQueue.front();
				int dstNbSamples = av_rescale_rnd(swr_get_delay(m_swrCtx, m_pCodecCtx->sample_rate) + aFrame->nb_samples,
					m_outSampleRate, m_pCodecCtx->sample_rate, AV_ROUND_UP);

				if (0 > av_samples_alloc(dstFrame->data, dstFrame->linesize, m_outChannels, dstNbSamples, m_outSampleFmt, 1))
					return;

				m_nbSamples = dstFrame->nb_samples;

				dstFrame->nb_samples = swr_convert(m_swrCtx, dstFrame->data, dstNbSamples, (const uint8_t**)aFrame->data, aFrame->nb_samples);
				if (dstFrame->nb_samples < 0)
					return;

				int wSize = av_audio_fifo_write(m_audioFifo, (void**)dstFrame->data, dstFrame->nb_samples);
				if (wSize < dstFrame->nb_samples)
					return;
			}

			ReadFifoFrame();
			
			m_audioQueue.pop();

		}
	}	
}

wchar_t* CCapAudio::GetMicrophoneName()
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

void CCapAudio::PushFrame(AVFrame* frame)
{
	AVFrame* audioFrame = av_frame_clone(frame);
	m_audioQueue.push(audioFrame);
}

void CCapAudio::ReadFifoFrame()
{
	AVFrame* frame = av_frame_alloc();
	frame->nb_samples = m_nbSamples;
	frame->channel_layout = m_outChannelLayout;
	frame->format = m_outSampleFmt;
	frame->sample_rate = m_outSampleRate;
	frame->pts = m_nbSamples * m_frameIndex++;
	av_frame_get_buffer(frame, 0);

	av_audio_fifo_read(m_audioFifo, (void**)frame->data, m_nbSamples);
}

