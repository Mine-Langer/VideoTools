#include "CPlayer.h"

CPlayer::CPlayer()
{

}

CPlayer::~CPlayer()
{

}

bool CPlayer::Open(const char* szFile)
{
	if (0 != avformat_open_input(&FormatCtx, szFile, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(FormatCtx, nullptr))
		return false;

	VideoIndex = av_find_best_stream(FormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	AudioIndex = av_find_best_stream(FormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

	if (VideoIndex != -1) {
		m_videoDecoder.Open(FormatCtx->streams[VideoIndex]);
		m_videoDecoder.SetConfig(m_rect.w, m_rect.h, AV_PIX_FMT_YUV420P, SWS_BICUBIC);
	}

	if (AudioIndex != -1) {
		m_audioDecoder.Open(FormatCtx->streams[AudioIndex]);
		m_audioDecoder.SetConfig();
	}

	SrcPacket = av_packet_alloc();

	return true;
}

void CPlayer::Start()
{
	if (VideoIndex != -1)
	{
		m_videoDecoder.Start(this);
	}

	if (AudioIndex != -1)
	{
		m_audioDecoder.Start(this);
	}

	m_bRun = true;
	m_ReadThread = std::thread(&CPlayer::OnReadFunction, this); // 解码

	// 播放线程
	m_PlayThread = std::thread(&CPlayer::OnPlayFunction, this); 
}

bool CPlayer::InitWindow(const void* pwnd, int width, int height)
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	m_window = SDL_CreateWindowFrom(pwnd);
	if (m_window == nullptr)
		return false;

	m_rect.x = 0;
	m_rect.y = 0;
	m_rect.w = width;
	m_rect.h = height;

	m_render = SDL_CreateRenderer(m_window, -1, 0);
	if (m_render == nullptr)
		return false;

	m_texture = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (m_texture == nullptr)
		return false;

	m_audioSpec.freq = m_audioDecoder.GetSampleRate();
	m_audioSpec.format = AUDIO_S16SYS;
	m_audioSpec.channels = m_audioDecoder.GetChannels();
	m_audioSpec.silence = 0;
	m_audioSpec.samples = m_audioDecoder.GetSamples();
	m_audioSpec.userdata = this;
	m_audioSpec.callback = OnAudioCallback;
	
	if (SDL_OpenAudio(&m_audioSpec, nullptr) < 0)
		return false;

	SDL_PauseAudio(0);

	return true;
}

void CPlayer::OnReadFunction()
{
	int error = 0;
	while (m_bRun)
	{
		if (0 > av_read_frame(FormatCtx, SrcPacket))
			break;
		
		if (SrcPacket->stream_index == VideoIndex)
		{
			m_videoDecoder.SendPacket(SrcPacket);
		}

		else if (SrcPacket->stream_index == AudioIndex)
		{
			m_audioDecoder.SendPacket(SrcPacket);
		}

		av_packet_unref(SrcPacket);
	}
}

void CPlayer::OnPlayFunction()
{
	AVFrame* vFrame = nullptr;
	while (m_bRun)
	{
		if (!VideoFrameData.Pop(vFrame))
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			SDL_UpdateYUVTexture(m_texture, nullptr, vFrame->data[0], vFrame->linesize[0], vFrame->data[1], vFrame->linesize[1], vFrame->data[2], vFrame->linesize[2]);
			SDL_RenderClear(m_render);
			SDL_RenderCopy(m_render, m_texture, nullptr, &m_rect);

			std::this_thread::sleep_for(std::chrono::milliseconds(66));
			SDL_RenderPresent(m_render);		
			HL_PRINT("[VideoFrame] pts:%lld, dts:%lld \r\n", vFrame->pts, vFrame->pkt_dts);
			av_frame_free(&vFrame);
		}
	}
}

void CPlayer::VideoEvent(AVFrame* vdata)
{
	AVFrame* vframe = av_frame_clone(vdata);
	VideoFrameData.MaxSizePush(vframe);
}

void CPlayer::AudioEvent(AVFrame* adata)
{
	AVFrame* aframe = av_frame_clone(adata);
	AudioFrameData.MaxSizePush(aframe);
}

void CPlayer::OnAudioCallback(void* userdata, Uint8* stream, int len)
{

}
