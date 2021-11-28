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
	// 设置输出参数
	if (!InitOutput(szFile))
		return false;
	
	// 初始化摄像头
	// m_videoDecoder.InitCamera();

	// 设置视频参数
	if (!m_videoDecoder.OpenScreen(CapX, CapY, capWidth, capHeight))
		return false;
	
	if (!m_videoDecoder.Start(this))
		return false;

 	// 设置音频参数
	if (!m_audioDecoder.Init(AudioEncCodecCtx->sample_fmt, AudioEncCodecCtx->channels, 
		AudioEncCodecCtx->channel_layout, AudioEncCodecCtx->sample_rate, AudioEncCodecCtx->frame_size))
		return false;
	
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
		InitVideoOutput();

	if (OutputFormatCtx->oformat->audio_codec != AV_CODEC_ID_NONE)
		InitAudioOutput();

	av_dump_format(OutputFormatCtx, 0, szOutput, 1);

	return true;
}

void CRecorder::Stop()
{

}

bool CRecorder::VideoEvent(AVFrame* frame)
{
	AVFrame* vFrame = av_frame_clone(frame);
	if (vFrame == nullptr)
		return false;

	m_videoQueue.push(vFrame);

	return true;
}

bool CRecorder::AudioEvent(AVFrame* frame)
{
	AVFrame* aFrame = av_frame_clone(frame);
	if (aFrame == nullptr)
		return false;
	
	m_audioQueue.push(aFrame);
	
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

	// 录音解复用线程
	//m_demuxAThread = std::thread(&CRecorder::OnDemuxAudioThread, this);

	// 保存数据帧线程
	m_saveThread = std::thread(&CRecorder::OnSaveThread, this);

	return true;
}

void CRecorder::OnDemuxAudioThread()
{
	AVPacket packet = { 0 };
	AVFrame* srcFrame = av_frame_alloc();
	AVFrame* dstFrame = av_frame_alloc();
	dstFrame->format = AudioEncCodecCtx->sample_fmt;
	dstFrame->channel_layout = AudioEncCodecCtx->channel_layout ? AudioEncCodecCtx->channel_layout : AV_CH_LAYOUT_STEREO;
	dstFrame->sample_rate = AudioEncCodecCtx->sample_rate;
	dstFrame->nb_samples = m_nbSamples;
	av_frame_get_buffer(dstFrame, 0);
	int dstNbSamples = av_rescale_rnd(m_nbSamples, AudioEncCodecCtx->sample_rate, AudioCodecCtx->sample_rate, AV_ROUND_UP);
	int maxDstNbSamples = dstNbSamples;

	while (m_state != Stopped)
	{
		if (m_state == Paused)
		{
			std::unique_lock<std::mutex> lock(m_mutexPause);
			m_cvPause.wait(lock, [this] { return m_state != Paused; });
		}

		if (0 > av_read_frame(AudioFormatCtx, &packet))
			continue;

		if (packet.stream_index == VideoIndex)
		{
			if (0 > avcodec_send_packet(AudioCodecCtx, &packet))
			{
				av_packet_unref(&packet);
				continue;
			}

			if (0 > avcodec_receive_frame(AudioCodecCtx, srcFrame))
			{
				av_packet_unref(&packet);
				continue;
			}

			dstNbSamples = av_rescale_rnd(swr_get_delay(SwrCtx, AudioCodecCtx->sample_rate) + srcFrame->nb_samples,
				AudioEncCodecCtx->sample_rate, AudioCodecCtx->sample_rate, AV_ROUND_UP);
			if (dstNbSamples > maxDstNbSamples)
			{
				av_freep(&dstFrame->data[0]);
				if (0 > av_samples_alloc(dstFrame->data, dstFrame->linesize, 
					AudioEncCodecCtx->channels, dstNbSamples, AudioEncCodecCtx->sample_fmt, 1))
					return;

				maxDstNbSamples = dstNbSamples;
				AudioEncCodecCtx->frame_size = dstNbSamples;
				m_nbSamples = dstFrame->nb_samples;
			}

			dstFrame->nb_samples = swr_convert(SwrCtx, dstFrame->data, dstNbSamples, (const uint8_t**)srcFrame->data, srcFrame->nb_samples);
			if (dstFrame->nb_samples < 0)
				return;

			{
				std::unique_lock<std::mutex> lock(m_mutexAudioBuf);
				m_cvAudioBufNotFull.wait(lock, [=] { return av_audio_fifo_space(AudioFifo) >= dstFrame->nb_samples; });
			}
			if (dstFrame->nb_samples > av_audio_fifo_write(AudioFifo, (void**)dstFrame->data, dstFrame->nb_samples))
				return;
			m_cvAudioBufNotEmpty.notify_one();
		}
		

		av_packet_unref(&packet);
	}
}

void CRecorder::OnSaveThread()
{
	AVPacket* vdata = nullptr;
	AVPacket* adata = nullptr;

	bool done = false;
	while (true)
	{	
		if (m_state == Stopped && !done)
			done = true;

		if (done)
		{
			std::unique_lock<std::mutex> vLock(m_mutexVideoBuf, std::defer_lock);
			std::unique_lock<std::mutex> aLock(m_mutexAudioBuf, std::defer_lock);
			std::lock(vLock, aLock);
			
			if (av_fifo_size(VideoFifo) < m_nImageSize && av_audio_fifo_size(AudioFifo) < m_nbSamples)
				break;
		}
		if (av_compare_ts(m_vCurPts, OutputFormatCtx->streams[VideoIndex]->time_base,
			m_aCurPts, OutputFormatCtx->streams[AudioIndex]->time_base) <= 0)
		{
			if (done)
			{
				std::lock_guard<std::mutex> lock(m_mutexVideoBuf);
				if (av_fifo_size(VideoFifo) < m_nImageSize)
					continue;
			}
			else
			{
				std::unique_lock<std::mutex> lock(m_mutexVideoBuf);
				m_cvVideoBufNotFull.wait(lock, [this] {return av_fifo_size(VideoFifo) >= m_nImageSize; });
			}

			av_fifo_generic_read(VideoFifo, VideoBuffer, m_nImageSize, nullptr);
			m_cvVideoBufNotFull.notify_one();

		}
		else
		{

		}
	}
	av_write_trailer(OutputFormatCtx);
}

bool CRecorder::InitVideoOutput()
{
	int VideoWidth, VideoHeight; 
	AVPixelFormat VideoFormat;

	AVStream* pVideoStream = avformat_new_stream(OutputFormatCtx, nullptr);
	if (pVideoStream == nullptr)
		return false;
	
	pVideoStream->time_base = { 1, 25 };
	pVideoStream->id = OutputFormatCtx->nb_streams - 1;

	AVCodec* codec = avcodec_find_encoder(OutputFormatCtx->oformat->video_codec);
	if (codec == nullptr)
		return false;

	VideoEncCodecCtx = avcodec_alloc_context3(codec);
	if (VideoEncCodecCtx == nullptr)
		return false;

	VideoEncCodecCtx->codec_id = OutputFormatCtx->oformat->video_codec;
	VideoEncCodecCtx->bit_rate = 4000000;
	VideoEncCodecCtx->width = capWidth;
	VideoEncCodecCtx->height = capHeight;
	VideoEncCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	VideoEncCodecCtx->time_base = pVideoStream->time_base;
	VideoEncCodecCtx->gop_size = 12;

	if (OutputFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		VideoEncCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	AVDictionary* options = nullptr;
	if (VideoEncCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		av_dict_set(&options, "preset", "slow", 0);
		av_dict_set(&options, "tune", "zerolatency", 0);
	}

	if (0 > avcodec_open2(VideoEncCodecCtx, codec, &options))
		return false;

	if (0 > avcodec_parameters_from_context(pVideoStream->codecpar, VideoEncCodecCtx))
		return false;

	/*m_nImageSize = av_image_get_buffer_size(VideoEncCodecCtx->pix_fmt, capWidth, capHeight, 1);
	VideoBuffer = (uint8_t*)av_malloc(m_nImageSize);
	VideoFrame = av_frame_alloc();
	av_image_fill_arrays(VideoFrame->data, VideoFrame->linesize, VideoBuffer, VideoEncCodecCtx->pix_fmt, capWidth, capHeight, 1);

	// 申请30帧缓存
	VideoFifo = av_fifo_alloc_array(30, m_nImageSize);
	if (VideoFifo == nullptr)
		return false;*/

	return true;
}

bool CRecorder::InitAudioOutput()
{
	AVStream* pAudioStream = avformat_new_stream(OutputFormatCtx, nullptr);
	if (pAudioStream == nullptr)
		return false;

	pAudioStream->id = OutputFormatCtx->nb_streams - 1;
	AVCodec* codec = avcodec_find_encoder(OutputFormatCtx->oformat->audio_codec);
	if (codec == nullptr)
		return false;

	AudioEncCodecCtx = avcodec_alloc_context3(codec);
	if (AudioEncCodecCtx == nullptr)
		return false;

	AudioEncCodecCtx->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	AudioEncCodecCtx->bit_rate = 64000;
	AudioEncCodecCtx->sample_rate = 44100;
	AudioEncCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	AudioEncCodecCtx->channels = av_get_channel_layout_nb_channels(AudioEncCodecCtx->channel_layout);
	pAudioStream->time_base = { 1, AudioEncCodecCtx->sample_rate };

	if (OutputFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		AudioEncCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (0 > avcodec_open2(AudioEncCodecCtx, codec, nullptr))
		return false;

	if (0 > avcodec_parameters_from_context(pAudioStream->codecpar, AudioEncCodecCtx))
		return false;

	/*SwrCtx = swr_alloc_set_opts(nullptr, AudioEncCodecCtx->channel_layout, AudioEncCodecCtx->sample_fmt, AudioEncCodecCtx->sample_fmt,
		AudioCodecCtx->channel_layout, AudioCodecCtx->sample_fmt, AudioCodecCtx->sample_rate, 0, nullptr);
	if (SwrCtx == nullptr)
		return false;

	av_opt_set_int(SwrCtx, "in_channel_count", AudioCodecCtx->channels, 0);
	av_opt_set_int(SwrCtx, "out_channel_count", AudioEncCodecCtx->channels, 0);

	if (0 > swr_init(SwrCtx))
		return false;

	// 设置缓存
	m_nbSamples = AudioEncCodecCtx->frame_size;
	if (!m_nbSamples)
		m_nbSamples = 1024;

	AudioFifo = av_audio_fifo_alloc(AudioEncCodecCtx->sample_fmt, AudioEncCodecCtx->channels, 30 * m_nbSamples);
	if (!AudioFifo)
		return false;*/

	return true;
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


