#include "Composite.h"

Composite::Composite()
{

}

Composite::~Composite()
{

}

void Composite::AddAudio(const char* szFile)
{
	//GetAudioImage(szFile);

	//strcpy_s(m_szAudioFile, sizeof(m_szAudioFile), szFile);
}

void Composite::AddImage(const char* szFile)
{
	//strcpy_s(m_szVideoFile, sizeof(m_szVideoFile), szFile);
}

bool Composite::OpenImage(const char* szFile)
{
	if (!m_videoDecoder.Open(szFile))
		return false;

	//if (m_type == E_Play)
	//	m_videoDecoder.SetSwsConfig(&m_rect, m_videoWidth, m_videoHeight);
	//else if (m_type == E_Save)
	//	m_videoDecoder.SetSwsConfig(&m_rect);
	//m_videoDecoder.Start(this);

	return true;
}

bool Composite::OpenAudio(const char* szFile)
{
	if (!m_audioDecoder.Open(szFile))
		return false;

	int sample_rate = -1;
	AVChannelLayout ch_layout = { };
	enum AVSampleFormat sample_fmt = AV_SAMPLE_FMT_NONE;
	m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
	m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);
	m_audioDecoder.Start(this);

	// 播放参数
	//m_audioSpec.freq = sample_rate;
	//m_audioSpec.format = AUDIO_S16SYS;
	//m_audioSpec.channels = av_get_channel_layout_nb_channels(ch_layout);
	//m_audioSpec.silence = 0;
	//m_audioSpec.samples = nb_sample;
	//m_audioSpec.userdata = this;
	//m_audioSpec.callback = OnSDLAudioFunction;

	return true;
}

void Composite::Start()
{
	
}

void Composite::Close()
{
	while (!m_videoQueue.Empty())
	{
		AVFrame* vdata = nullptr;
		if (m_videoQueue.Pop(vdata))
			av_frame_free(&vdata);
	}

	m_videoDecoder.Stop();
}

void Composite::Play(std::vector<ItemElem>& vecImage, std::vector<ItemElem>& vecMusic)
{
	//m_type = E_Play;
	//if (m_state == NotStarted)
	//{
	//	// 打开音频
	//	if (!OpenAudio(m_szAudioFile))
	//		return;
	//	// 打开图像
	//	if (!OpenImage(m_szVideoFile))
	//		return;
	//	// 置为开始播放
	//	m_state = Started;
	//	if (0 > SDL_OpenAudio(&m_audioSpec, nullptr))
	//		return;
	//
	//	SDL_PauseAudio(0);
	//	m_playThread = std::thread(&Composite::OnPlayFunction, this);
	//}
	//else if (m_state == Started)
	//{
	//	// 置为暂停状态
	//	SDL_PauseAudio(1);
	//	m_state = Paused;
	//}
	//else if (m_state == Paused)
	//{
	//	SDL_PauseAudio(0);
	//	m_state = Started;
	//}
	for (int i = 0; i < vecMusic.size(); i++)
	{
		std::string strFile = vecMusic[i].filename.toStdString();
		m_demuxMusic.Open(strFile.c_str());
		
		m_audioDecoder.Open(&m_demuxMusic);

		m_demuxMusic.Start(this);

		m_audioDecoder.Start(this);

		m_player.Start();
	}

}

bool Composite::InitWnd(void* pWnd, int width, int height)
{
	//m_window = SDL_CreateWindowFrom(pWnd);
	//if (!m_window)
	//	return false;
	//
	//m_render = SDL_CreateRenderer(m_window, -1, 0);
	//if (!m_render)
	//	return false;
	//
	//m_texture = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
	//if (!m_texture)
	//	return false;
	//
	//m_videoWidth = width;
	//m_videoHeight = height;
	//m_rect.x = m_rect.y = 0;
	//m_rect.w = m_videoWidth;
	//m_rect.h = m_videoHeight;

	return true;
}

bool Composite::SaveFile(const char* szOutput, int type)
{
	m_type = E_Save;

	if (!OpenAudio(m_szAudioFile))
		return false;

	if (!OpenImage(m_szVideoFile))
		return false;

	if (0 > avformat_alloc_output_context2(&m_pOutFormatCtx, nullptr, nullptr, szOutput))
		return false;

	int srcWidth, srcHeight;
	AVPixelFormat srcFormat;
	m_videoDecoder.GetSrcParameter(srcWidth, srcHeight, srcFormat);

	m_bitRate = 400000;
	m_frameRate = 25;
	if (m_pOutFormatCtx->oformat->video_codec != AV_CODEC_ID_NONE && ((type & 0x1) == 0x1))
		if (!m_videoEncoder.Init(m_pOutFormatCtx, m_pOutFormatCtx->oformat->video_codec, srcWidth, srcHeight))
			return false;
	
	int sample_rate;
	AVChannelLayout ch_layout;
	enum AVSampleFormat sample_fmt;
	m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);

	if (m_pOutFormatCtx->oformat->audio_codec != AV_CODEC_ID_NONE && ((type & 0x2) == 0x2))
	{
		if (!m_audioEncoder.InitAudio(m_pOutFormatCtx, m_pOutFormatCtx->oformat->audio_codec, ch_layout, sample_fmt, sample_rate))
			return false;

		m_audioEncoder.BindAudioData(&m_audioQueue);
	}

	av_dump_format(m_pOutFormatCtx, 0, szOutput, 1);

	// 打开输出文件
	if (!(m_pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
		if (0 > avio_open(&m_pOutFormatCtx->pb, szOutput, AVIO_FLAG_WRITE))
			return false;

	// 写文件流头
	if (0 > avformat_write_header(m_pOutFormatCtx, nullptr))
		return false;

	// 开始写文件线程
	m_saveThread = std::thread(&Composite::OnSaveFunction, this);

	return true;
}

// 调试用  获取音频的图像数据
bool Composite::GetAudioImage(const char* filename)
{
	AVFormatContext* fmt_ctx = nullptr;
	AVCodecContext* codec_ctx = nullptr;
	if (0 != avformat_open_input(&fmt_ctx, filename, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(fmt_ctx, nullptr))
		return false;

	int img_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (img_idx < 0)
		return false;

	codec_ctx = avcodec_alloc_context3(nullptr);
	if (0 > avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[img_idx]->codecpar))
		return false;

	const AVCodec* pCodec = avcodec_find_decoder(codec_ctx->codec_id);
	if (!pCodec)
		return false;

	if (0 > avcodec_open2(codec_ctx, pCodec, nullptr))
		return false;

	int ret = 0;
	AVPacket packet;
	AVFrame frame;
	while (true)
	{
		ret = av_read_frame(fmt_ctx, &packet);
		if (ret < 0)
			break;
		if (packet.stream_index == img_idx)
		{
			avcodec_send_packet(codec_ctx, &packet);

			avcodec_receive_frame(codec_ctx, &frame);


		}
		av_packet_unref(&packet);
	}

	return true;
}

bool Composite::VideoEvent(AVFrame* frame)
{
	AVFrame* vdata = av_frame_clone(frame);
	bool bRun = (m_state != Stopped);
	m_videoQueue.MaxSizePush(vdata, &bRun);
	return true;
}

bool Composite::AudioEvent(AVFrame* frame)
{
	bool bRun = (m_state != Stopped);
	m_audioQueue.MaxSizePush(frame, &bRun);
	return true;
}



bool Composite::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_VIDEO)
	{
		m_videoDecoder.SendPacket(pkt);
	}
	else if (type == AVMEDIA_TYPE_AUDIO)
	{
		m_audioDecoder.SendPacket(pkt);
	}
	return true;
}

void Composite::CleanPacket()
{

}

void Composite::OnPlayFunction()
{

}

void Composite::OnSaveFunction()
{
	AVPacket* vPacket = nullptr;
	AVPacket* aPacket = av_packet_alloc();
	av_init_packet(aPacket);
	aPacket->pts = 0;

	int videoIndex = 0;
	int audioIndex = 0;
	int ret = 0;
	bool bRun = true;

	while (bRun)
	{
		if (m_videoQueue.Empty() || m_audioQueue.Empty())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		else
		{
			AVFrame* videoFrame = m_videoQueue.Front();
			if (videoFrame == nullptr)
				break;
			// 判断音频帧/视频帧时序
			if (0 >= av_compare_ts(videoIndex, m_videoEncoder.GetTimeBase(), audioIndex, m_audioEncoder.GetTimeBase()))
			{
				// 写视频帧
				AVFrame* swsVideoFrame = m_videoDecoder.ConvertFrame(videoFrame);
				if (!swsVideoFrame)
					continue;
				swsVideoFrame->pts = videoIndex++;

				vPacket = m_videoEncoder.Encode(swsVideoFrame);
				if (vPacket)
				{
					av_interleaved_write_frame(m_pOutFormatCtx, vPacket);
					av_packet_free(&vPacket);
				}

				av_frame_free(&swsVideoFrame);
			}
			else
			{
				// 写音频帧
				if (m_audioEncoder.GetEncodePacket(aPacket, audioIndex))
				{
					av_interleaved_write_frame(m_pOutFormatCtx, aPacket);
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
	av_write_trailer(m_pOutFormatCtx);
}
