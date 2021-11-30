#include "Composite.h"

Composite::Composite()
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

Composite::~Composite()
{

}

void Composite::AddAudio(const char* szFile)
{
	strcpy_s(m_szAudioFile, sizeof(m_szAudioFile), szFile);
}

void Composite::AddImage(const char* szFile)
{
	strcpy_s(m_szVideoFile, sizeof(m_szVideoFile), szFile);
}

bool Composite::OpenImage(const char* szFile)
{
	if (!m_videoDecoder.Open(szFile))
		return false;

	if (m_type == 0)
		m_videoDecoder.SetSwsConfig(m_videoWidth, m_videoHeight);
	else if (m_type == 1)
		m_videoDecoder.SetSwsConfig();
	m_videoDecoder.Start(this);

	return true;
}

bool Composite::OpenAudio(const char* szFile)
{
	if (!m_audioDecoder.Open(szFile))
		return false;

	int sample_rate = -1;
	int nb_sample = -1; 
	int64_t ch_layout = -1; 
	enum AVSampleFormat sample_fmt = AV_SAMPLE_FMT_NONE;
	m_audioDecoder.GetSrcParameter(sample_rate, nb_sample, ch_layout, sample_fmt);
	m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);
	m_audioDecoder.Start(this);

	// 播放参数
	m_audioSpec.freq = sample_rate;
	m_audioSpec.format = AUDIO_S16SYS;
	m_audioSpec.channels = av_get_channel_layout_nb_channels(ch_layout);
	m_audioSpec.silence = 0;
	m_audioSpec.samples = nb_sample;
	m_audioSpec.userdata = this;
	m_audioSpec.callback = OnSDLAudioFunction;

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

	m_videoDecoder.Release();
}

void Composite::Play()
{
	m_type = 0;
	if (m_state == NotStarted)
	{
		// 打开音频
		if (!OpenAudio(m_szAudioFile))
			return;
		// 打开图像
		if (!OpenImage(m_szVideoFile))
			return;
		// 置为开始播放
		m_state = Started;
		SDL_PauseAudio(0);
		if (0 > SDL_OpenAudio(&m_audioSpec, nullptr))
			return;

		m_playThread = std::thread(&Composite::OnPlayFunction, this);
	}
	else if (m_state == Started)
	{
		// 置为暂停状态
		m_state = Paused;
	}
	else if (m_state == Paused)
	{
		m_state = Started;
	}
}

bool Composite::InitWnd(void* pWnd, int width, int height)
{
	m_window = SDL_CreateWindowFrom(pWnd);
	if (!m_window)
		return false;

	m_render = SDL_CreateRenderer(m_window, -1, 0);
	if (!m_render)
		return false;

	m_texture = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!m_texture)
		return false;

	m_videoWidth = width;
	m_videoHeight = height;
	m_rect.x = m_rect.y = 0;
	m_rect.w = m_videoWidth;
	m_rect.h = m_videoHeight;

	return true;
}

bool Composite::SaveFile(const char* szOutput, int type)
{
	m_type = 1;
	m_audioDecoder.SetSaveEnable(true);
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
	
	if (m_pOutFormatCtx->oformat->audio_codec != AV_CODEC_ID_NONE && ((type & 0x2) == 0x2))
		InitAudioEnc(m_pOutFormatCtx->oformat->audio_codec);

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

bool Composite::VideoEvent(AVFrame* frame)
{
	AVFrame* vdata = av_frame_clone(frame);
	m_videoQueue.MaxSizePush(vdata);
	return true;
}

bool Composite::AudioEvent(AVFrame* frame)
{
	m_audioQueue.MaxSizePush(frame);
	return true;
}

bool Composite::InitAudioEnc(enum AVCodecID codec_id)
{
	AVCodec* pCodec = avcodec_find_encoder(codec_id);
	if (!pCodec)
		return false;

	m_pOutACodecCtx = avcodec_alloc_context3(pCodec);
	if (!m_pOutACodecCtx)
		return false;

	m_pOutACodecCtx->codec_id = codec_id;
	m_pOutACodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	m_pOutACodecCtx->channels = av_get_channel_layout_nb_channels(m_pOutACodecCtx->channel_layout);
	m_pOutACodecCtx->sample_fmt = pCodec->sample_fmts ? pCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	m_pOutACodecCtx->sample_rate = 44100;
	m_pOutACodecCtx->bit_rate = 96000;
	m_pOutACodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	m_pOutACodecCtx->time_base = { 1, 44100 };

	m_pOutAStream = avformat_new_stream(m_pOutFormatCtx, nullptr);
	m_pOutAStream->time_base = m_pOutACodecCtx->time_base;
	m_pOutAStream->id = m_pOutFormatCtx->nb_streams - 1;

	if (m_pOutFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		m_pOutACodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	
	if (0 > avcodec_open2(m_pOutACodecCtx, pCodec, nullptr))
		return false;

	if (0 > avcodec_parameters_from_context(m_pOutAStream->codecpar, m_pOutACodecCtx))
		return false;

	int sample_rate;
	int nb_sample; 
	int64_t ch_layout;
	enum AVSampleFormat sample_fmt;
	m_audioDecoder.GetSrcParameter(sample_rate, nb_sample, ch_layout, sample_fmt);
	m_pSwrCtx = swr_alloc_set_opts(nullptr, m_pOutACodecCtx->channel_layout, m_pOutACodecCtx->sample_fmt, m_pOutACodecCtx->sample_rate,
		ch_layout, sample_fmt, sample_rate, 0, nullptr);

	if (0 > swr_init(m_pSwrCtx))
		return false;

	m_pAudioFifo = av_audio_fifo_alloc(m_pOutACodecCtx->sample_fmt, m_pOutACodecCtx->channels, 1);
	if (!m_pAudioFifo)
		return false;
	m_audioFrameSize = m_pOutACodecCtx->frame_size;

	return true;
}

void Composite::OnSDLAudioFunction(void* userdata, Uint8* stream, int len)
{
	Composite* pThis = (Composite*)userdata;
	
	if (pThis->m_state != Started)
		return;

	if (pThis->m_audioQueue.Empty())
	{
		SDL_memset(stream, 0, len);
	}
	else
	{
		AVFrame* frame = nullptr;
		pThis->m_audioQueue.Pop(frame);
		if (frame != nullptr)
		{
			int wLen = len < frame->linesize[0] ? len : frame->linesize[0];
			SDL_memset(stream, 0, len);
			SDL_MixAudio(stream, frame->data[0], wLen, SDL_MIX_MAXVOLUME);

			av_frame_free(&frame);
		}
	}
	
}

void Composite::OnPlayFunction()
{
	while (m_state != Stopped)
	{
		if (m_state == Paused)
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		else
		{
			if (m_videoQueue.Empty())
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			else
			{
				AVFrame* frame = m_videoQueue.Front();
				AVFrame* swsFrame = m_videoDecoder.GetConvertFrame(frame);

				SDL_UpdateYUVTexture(m_texture, nullptr, swsFrame->data[0], swsFrame->linesize[0],
					swsFrame->data[1], swsFrame->linesize[1], swsFrame->data[2], swsFrame->linesize[2]);
				SDL_RenderClear(m_render);
				SDL_RenderCopy(m_render, m_texture, nullptr, &m_rect);
				SDL_RenderPresent(m_render);

				std::this_thread::sleep_for(std::chrono::milliseconds(40));

				av_frame_free(&swsFrame);
			}
		}
	}
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
			if (0 >= av_compare_ts(videoIndex, m_pOutVCodecCtx->time_base, audioIndex, m_pOutACodecCtx->time_base))
			{
				// 写视频帧
				AVFrame* swsVideoFrame = m_videoDecoder.GetConvertFrame(videoFrame);
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
				while (bRun && av_audio_fifo_size(m_pAudioFifo) < m_audioFrameSize)
				{
					AVFrame* audioFrame = nullptr;
					if (!m_audioQueue.Pop(audioFrame))
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						continue;
					}
					if (audioFrame == nullptr)
						bRun = false;
					else
					{
						uint8_t** convertedBuffer = new uint8_t* [m_pOutACodecCtx->channels]();
						av_samples_alloc(convertedBuffer, nullptr, m_pOutACodecCtx->channels, audioFrame->nb_samples, m_pOutACodecCtx->sample_fmt, 0);
						swr_convert(m_pSwrCtx, convertedBuffer, audioFrame->nb_samples, (const uint8_t**)audioFrame->extended_data, audioFrame->nb_samples);
						av_audio_fifo_realloc(m_pAudioFifo, av_audio_fifo_size(m_pAudioFifo) + audioFrame->nb_samples);
						av_audio_fifo_write(m_pAudioFifo, (void**)convertedBuffer, audioFrame->nb_samples);
						av_freep(&convertedBuffer[0]);
						delete[] convertedBuffer;
						av_frame_free(&audioFrame);
					}
				}
				
				if (!bRun)
					break;

				while (av_audio_fifo_size(m_pAudioFifo) >= m_audioFrameSize || (!bRun && av_audio_fifo_size(m_pAudioFifo)>0))
				{
					const int frameSize = FFMIN(av_audio_fifo_size(m_pAudioFifo), m_audioFrameSize);
					AVFrame* outputFrame = av_frame_alloc();
					outputFrame->nb_samples = frameSize;
					outputFrame->channel_layout = m_pOutACodecCtx->channel_layout;
					outputFrame->format = m_pOutACodecCtx->sample_fmt;
					outputFrame->sample_rate = m_pOutACodecCtx->sample_rate;
					av_frame_get_buffer(outputFrame, 0);

					av_audio_fifo_read(m_pAudioFifo, (void**)outputFrame->data, frameSize);
					outputFrame->pts = audioIndex;
					audioIndex += outputFrame->nb_samples;

					if (0 > avcodec_send_frame(m_pOutACodecCtx, outputFrame))
						break;

					ret = avcodec_receive_packet(m_pOutACodecCtx, aPacket);
					if (ret == 0)
					{
						av_packet_rescale_ts(aPacket, m_pOutACodecCtx->time_base, m_pOutAStream->time_base);
						aPacket->stream_index = m_pOutAStream->index;
						printf(" audio packet pts: %I64d.\r\n", aPacket->pts);

						av_interleaved_write_frame(m_pOutFormatCtx, aPacket);
					}
					av_frame_free(&outputFrame);
					av_packet_unref(aPacket);
				}
			}
		}
	}
	av_write_trailer(m_pOutFormatCtx);
}
