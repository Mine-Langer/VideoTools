#include "CPlayer.h"

CPlayer::CPlayer()
{
	avformat_network_init();
}

CPlayer::~CPlayer()
{
	avformat_network_deinit();
}

bool CPlayer::Open(const char* szFile)
{
	if (!m_demux.Open(szFile))
		return false;
	
	AVStream* videoStream = m_demux.VideoStream();
	if (videoStream)
	{
		m_videoDecoder.Open(videoStream);
		m_videoDecoder.SetConfig(m_rect, m_rect.w, m_rect.h);
	}

	AVStream* audioStream = m_demux.AudioStream();
	if (audioStream) 
	{
		if (m_audioDecoder.Open(audioStream))
			InitAudio();
	}
	
	return true;
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

	return true;
}

void CPlayer::Start(IPlayerEvent* pEvent)
{
	m_pEvent = pEvent;
	m_demux.Start(this);

	m_videoDecoder.Start(this);

	m_audioDecoder.Start(this);
	SDL_PauseAudio(0);

	m_avStatus = ePlay;
	m_bPlayOvered = false;
	m_playThread = std::thread(&CPlayer::OnPlayFunction, this);
}

bool CPlayer::InitAudio()
{
	m_audioDecoder.InitAudioSpec(m_audioSpec);
	m_audioSpec.format = AUDIO_S16SYS;
	m_audioSpec.silence = 0;
	m_audioSpec.userdata = this;
	m_audioSpec.callback = OnAudioCallback;

	if (SDL_OpenAudio(&m_audioSpec, nullptr) < 0)
		return false;

	SDL_PauseAudio(1);
	
	return true;
}

void CPlayer::UpdateWindow(int width, int height)
{
	m_rect.w = width;
	m_rect.h = height;
	if (m_avStatus != ePlay)
		return;

	m_videoDecoder.SetConfig(m_rect, m_rect.w, m_rect.h);

	if (m_texture)
	{
		SDL_DestroyTexture(m_texture);
		m_texture = nullptr;
	}

	m_texture = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
}

void CPlayer::UpdateWindow(int x, int y, int width, int height)
{
	m_rect.x = x;
	m_rect.y = y;
	m_rect.w = width;
	m_rect.h = height;
}

void CPlayer::Close()
{
	m_avStatus = eStop;
	if (m_playThread.joinable())
		m_playThread.join();

	SDL_CloseAudio();
	m_videoDecoder.Close();
	m_audioDecoder.Close();

	m_demux.Close();

	if (m_pEvent)
		m_pEvent->OnPlayStatus(m_avStatus);
}

void CPlayer::Release()
{
	m_avStatus = eStop;

	m_demux.Close();

	m_videoDecoder.Close();

	m_audioDecoder.Close();

	if (m_pEvent)
		m_pEvent->OnPlayStatus(m_avStatus);
}

void CPlayer::OnPlayFunction()
{
	AVFrame* vFrame = nullptr;
	while (m_avStatus != eStop)
	{
		if (!m_videoQueue.Pop(vFrame))
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			if (vFrame == nullptr)
			{
				SDL_RenderClear(m_render);
				SDL_RenderPresent(m_render);
				m_bPlayOvered = true;
				break;
			}

			vFrame = m_videoDecoder.ConvertFrame(vFrame);

			printf("\timage frame: pts(%lld), time_pos(%lld) \r\n", vFrame->pts, vFrame->pkt_dts);

			SDL_UpdateYUVTexture(m_texture, nullptr, vFrame->data[0], vFrame->linesize[0], vFrame->data[1], vFrame->linesize[1], vFrame->data[2], vFrame->linesize[2]);
			SDL_RenderClear(m_render);
			SDL_RenderCopy(m_render, m_texture, nullptr, &m_rect);

// 			int64_t delay = m_sync.CalcDelay(vFrame->pts * m_videoDecoder.GetTimebase());
// 			if (delay > 0)
// 				std::this_thread::sleep_for(std::chrono::microseconds(delay));
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
			SDL_RenderPresent(m_render);		
//			HL_PRINT("[VideoFrame] pts:%lld, dts:%lld \r\n", vFrame->pts, vFrame->pkt_dts);
			av_frame_free(&vFrame);
//			m_sync.SetShowTime();
		}
	}
	if (m_bPlayOvered)
		Release();
}


bool CPlayer::OnDemuxPacket(AVPacket* pkt, int type)
{
	switch (type)
	{
	case AVMEDIA_TYPE_VIDEO:
		m_videoDecoder.PushPacket(pkt);
		break;
	case AVMEDIA_TYPE_AUDIO:
		m_audioDecoder.PushPacket(pkt);
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		break;
	}
	return true;
}

bool CPlayer::VideoEvent(AVFrame* vdata)
{
	bool bRun = (m_avStatus == eStop);
	m_videoQueue.MaxSizePush(vdata, &bRun);
	return true;
}

bool CPlayer::AudioEvent(AVFrame* adata)
{
	bool bRun = (m_avStatus == eStop);
	m_audioQueue.MaxSizePush(adata, &bRun);
	return true;
}

void CPlayer::OnAudioCallback(void* userdata, Uint8* stream, int len)
{
	CPlayer* pThis = (CPlayer*)userdata;
	if (pThis->m_avStatus != ePlay)
		return;

	AVFrame* aframe = nullptr;
	if (pThis->m_audioQueue.Pop(aframe))
	{
		int wlen = len < aframe->nb_samples ? len : aframe->nb_samples;
		SDL_memset(stream, 0, wlen);
		SDL_MixAudio(stream, aframe->data[0], wlen, SDL_MIX_MAXVOLUME);
		//pThis->m_sync.SetAudioClock(aframe->dpts);
		//pThis->m_playEvent->UpdatePlayPosition(aframe->dpts);
		//HL_PRINT("	[AudioFrame] pts:%lld, dts:%f \r\n", aframe->pts, aframe->dpts);
		av_frame_free(&aframe);
	}
	else
	{
		SDL_memset(stream, 0, len);
	}
}
