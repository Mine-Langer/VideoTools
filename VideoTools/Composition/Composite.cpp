#include "Composite.h"

Composite::Composite()
{

}

Composite::~Composite()
{

}


bool Composite::OpenImage(std::vector<ItemElem>& vecImage)
{
	for (int i = 0; i < vecImage.size(); i++)
	{
		std::string szFilename = vecImage[i].filename.toStdString();
		CImageFrame img_frame;
		if (img_frame.Open(szFilename.c_str()))
			m_videoQueue.Push(img_frame.ImageFrame());
	}

	return true;
}

bool Composite::OpenAudio(std::vector<ItemElem>& vecAudio)
{
	bool bRet = false;
	AVFrame* frame = nullptr;
	for (int i = 0; i < vecAudio.size(); i++)
	{
		std::string szFilename = vecAudio[i].filename.toStdString();
		CAudioFrame audio_frame;
		audio_frame.Open(szFilename.c_str(), vecAudio[i].start, vecAudio[i].end);
		while (true)
		{
			frame = audio_frame.AudioFrame(bRet);
			if (frame == nullptr && bRet)
				break;
			else if (frame != nullptr && bRet)
			{
				bRet = true;
				m_audioQueue.MaxSizePush(frame, &bRet);
			}	
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}	
	}
	m_audioQueue.Push(nullptr);

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
	m_nType = 1; // 播放

	m_player.SetView(m_hWndView, m_videoWidth, m_videoHeight);
	SDL_Rect rect;
	for (int i = 0; i < vecImage.size(); i++)
	{
		std::string strFile = vecMusic[i].filename.toStdString();
		m_demuxImage.Open(strFile.c_str());
		if (m_videoDecoder.Open(&m_demuxImage))
		{
			m_videoDecoder.SetSwsConfig(&rect, m_videoWidth, m_videoHeight);

			m_demuxImage.Start(this);

			m_videoDecoder.Start(this);
		}
	}

	for (int i = 0; i < vecMusic.size(); i++)
	{
		std::string strFile = vecMusic[i].filename.toStdString();
		m_demuxMusic.Open(strFile.c_str());
		
		if (m_audioDecoder.Open(&m_demuxMusic))
		{
			int sample_rate;
			AVChannelLayout ch_layout;
			AVSampleFormat sample_fmt;
			m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);

			int nbSamples = m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);

			m_player.SetAudioSpec(sample_rate, ch_layout, nbSamples);
		}

		m_demuxMusic.Start(this);

		m_audioDecoder.Start(this);
		
		m_player.PlayAudio();
	}

}

bool Composite::InitWnd(HWND pWnd, int width, int height)
{
	m_hWndView = pWnd;
	m_videoWidth = width;
	m_videoHeight = height;

	return true;
}

bool Composite::SaveFile(const char* szOutput, std::vector<ItemElem>& vecImage, std::vector<ItemElem>& vecMusic)
{
	m_vecImage = vecImage;
	m_vecMusic = vecMusic;

	if (0 > avformat_alloc_output_context2(&m_pOutFormatCtx, nullptr, nullptr, szOutput))
		return false;

	m_bitRate = 4000000;
	m_frameRate = 25;
	if (m_pOutFormatCtx->oformat->video_codec != AV_CODEC_ID_NONE)
		if (!m_videoEncoder.Init(m_pOutFormatCtx, AV_CODEC_ID_H264, m_outputWidth, m_outputHeight))
			return false;
	
	int sample_rate = 44100;
	AVChannelLayout ch_layout;
	av_channel_layout_default(&ch_layout, 2);
	AVSampleFormat sample_fmt = AV_SAMPLE_FMT_S16;

	if (m_pOutFormatCtx->oformat->audio_codec != AV_CODEC_ID_NONE)
	{
		if (!m_audioEncoder.InitAudio(m_pOutFormatCtx, m_pOutFormatCtx->oformat->audio_codec))
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
	m_demuxThread = std::thread(&Composite::OnDemuxFunction, this);

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
	
	if (m_nType == 1)
		m_player.SendAudioFrane(frame);
	else
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
		if (m_videoQueue.Empty() ||  m_audioQueue.Empty())
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
				AVFrame* swsVideoFrame = videoFrame; // m_videoDecoder.ConvertFrame(videoFrame);
				if (!swsVideoFrame)
					continue;
				swsVideoFrame->pts = videoIndex++;

				vPacket = m_videoEncoder.Encode(swsVideoFrame);
				if (vPacket)
				{
					av_interleaved_write_frame(m_pOutFormatCtx, vPacket);
					av_packet_free(&vPacket);
				}
				//printf("write video frame index:%d\n", videoIndex);
				//av_frame_free(&swsVideoFrame);
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
				//printf("write audio frame index:%d\n", audioIndex);
			}
		}
	}
	av_write_trailer(m_pOutFormatCtx);
	avio_closep(&m_pOutFormatCtx->pb);
}

void Composite::OnDemuxFunction()
{
	OpenImage(m_vecImage);

	OpenAudio(m_vecMusic);
}



/*****************************************************************************************
******************************************************************************************/
CImageFrame::~CImageFrame()
{
	Release();
}

bool CImageFrame::Open(const char* szfile)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szfile, nullptr, nullptr))
		return false;
	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;
	int img_idx = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (0 > img_idx)
		return false;
	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (0 > avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[img_idx]->codecpar))
		return false;
	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;
	
	AVPacket pkt;
	while (true)
	{
		if (0 > av_read_frame(m_pFormatCtx, &pkt))
			break;
		if (pkt.stream_index == img_idx)
		{
			if (0 > avcodec_send_packet(m_pCodecCtx, &pkt))
				continue;

			m_pFrameData = av_frame_alloc();
			if (0 > avcodec_receive_frame(m_pCodecCtx, m_pFrameData)) {
				av_frame_free(&m_pFrameData);
				m_pFrameData = nullptr;
				continue;
			}
			break;
		}
	}

	if (!m_pFrameData)
		return false;

	return true;
}

AVFrame* CImageFrame::ImageFrame()
{
	return m_pFrameData;
}

void CImageFrame::Release()
{
	if (m_pFormatCtx) {
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}

	if (m_pCodecCtx) {
		avcodec_close(m_pCodecCtx);
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}
}

/********************************************************************************************
*********************************************************************************************/
CAudioFrame::~CAudioFrame()
{
	Release();
}

bool CAudioFrame::Open(const char* szfile, int begin, int end)
{
	if (0 != avformat_open_input(&m_pFormatCtx, szfile, nullptr, nullptr))
		return false;
	if (0 > avformat_find_stream_info(m_pFormatCtx, nullptr))
		return false;
	int audio_idx = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (0 > audio_idx)
		return false;
	m_pCodecCtx = avcodec_alloc_context3(nullptr);
	if (0 > avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[audio_idx]->codecpar))
		return false;
	const AVCodec* pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (0 > avcodec_open2(m_pCodecCtx, pCodec, nullptr))
		return false;
	
	m_begin_pts = begin * AV_TIME_BASE;
	m_end_pts = end * AV_TIME_BASE;

	m_timebase = av_q2d(m_pCodecCtx->time_base);
	m_audio_idx = audio_idx;

	m_swr_sample_rate = 44100;
	av_channel_layout_default(&m_swr_ch_layout, 2);
	m_swr_sample_fmt = AV_SAMPLE_FMT_S16;
	if (0 > swr_alloc_set_opts2(&m_swr_ctx, &m_swr_ch_layout, m_swr_sample_fmt, m_swr_sample_rate,
		&m_pCodecCtx->ch_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate, 0, nullptr))
		return false;

	if (0 > swr_init(m_swr_ctx))
		return false;

	m_bRun = true;
	m_tRead = std::thread(&CAudioFrame::OnRun, this);

	return true;
}

AVFrame* CAudioFrame::AudioFrame(bool& bstatus)
{
	AVFrame* frame = nullptr;
	bstatus = m_frameQueueData.Pop(frame);
	return frame;
}

void CAudioFrame::Release()
{
	if (m_tRead.joinable())
		m_tRead.join();

	if (m_pFormatCtx) {
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}

	if (m_pCodecCtx) {
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (m_swr_ctx) {
		swr_free(&m_swr_ctx);
		m_swr_ctx = nullptr;
	}
}

void CAudioFrame::OnRun()
{
	if (0 > avformat_seek_file(m_pFormatCtx, -1, INT_MIN, m_begin_pts, INT_MAX, 0))
		return;

	AVFrame* frame = av_frame_alloc();

	AVPacket pkt;
	while (m_bRun)
	{
		if (0 > av_read_frame(m_pFormatCtx, &pkt))
		{
			m_frameQueueData.MaxSizePush(nullptr, &m_bRun);
			break;
		}

		if (pkt.stream_index != m_audio_idx)
			continue;

		if (0 > avcodec_send_packet(m_pCodecCtx, &pkt))
			continue;

		if (0 > avcodec_receive_frame(m_pCodecCtx, frame))
		{
			av_frame_free(&frame);
			continue;
		}

		AVFrame* cvtFrame = av_frame_alloc();
		cvtFrame->sample_rate = m_swr_sample_rate;
		cvtFrame->format = m_swr_sample_fmt;
		cvtFrame->nb_samples = frame->nb_samples;
		av_frame_get_buffer(cvtFrame, 0);

		swr_convert(m_swr_ctx, cvtFrame->data, cvtFrame->nb_samples, (const uint8_t**)frame->extended_data, frame->nb_samples);

		m_frameQueueData.MaxSizePush(cvtFrame, &m_bRun);
		av_frame_unref(frame);
	}
}
