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
	if (audioStream) { }
	
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

void CPlayer::Start()
{
	m_demux.Start(this);

	m_videoDecoder.Start(this);

	m_avStatus = ePlay;
	m_playThread = std::thread(&CPlayer::OnPlayFunction, this);
	m_playThread.detach();
}

bool CPlayer::InitAudio()
{
// 	m_audioSpec.freq = m_audioDecoder.GetSampleRate();
// 	m_audioSpec.format = AUDIO_S16SYS;
// 	m_audioSpec.channels = m_audioDecoder.GetChannels();
// 	m_audioSpec.silence = 0;
// 	m_audioSpec.samples = m_audioDecoder.GetSamples();
// 	m_audioSpec.userdata = this;
// 	m_audioSpec.callback = OnAudioCallback;
// 
// 	if (SDL_OpenAudio(&m_audioSpec, nullptr) < 0)
// 		return false;
// 
// 	SDL_PauseAudio(0);
	
	return true;
}

void CPlayer::UpdateWindow(int width, int height)
{
	m_rect.w = width;
	m_rect.h = height;

	m_videoDecoder.SetConfig(m_rect, m_rect.w, m_rect.h);

	if (m_texture)
	{
		SDL_DestroyTexture(m_texture);
		m_texture = nullptr;
	}

	m_texture = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
}

void CPlayer::Close()
{
	m_avStatus = eStop;

	m_demux.Close();

	m_videoDecoder.Close();
}

void CPlayer::OnReadFunction()
{

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
				break;

			vFrame = m_videoDecoder.ConvertFrame(vFrame);

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

	Close();
}


bool CPlayer::OnDemuxPacket(AVPacket* pkt, int type)
{
	switch (type)
	{
	case AVMEDIA_TYPE_VIDEO:
		m_videoDecoder.PushPacket(pkt);
		break;
	case AVMEDIA_TYPE_AUDIO:
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		break;
	}
	return true;
}

bool CPlayer::VideoEvent(AVFrame* vdata)
{
	m_videoQueue.MaxSizePush(vdata);
	return true;
}

bool CPlayer::AudioEvent(AVFrame* adata)
{
	return true;
}

void CPlayer::OnAudioCallback(void* userdata, Uint8* stream, int len)
{
// 	CPlayer* pThis = (CPlayer*)userdata;
// 	STAudioBuffer* aframe = nullptr;
// 	if (pThis->AudioFrameData.Pop(aframe))
// 	{
// 		int wlen = len < aframe->size ? len : aframe->size;
// 		SDL_memset(stream, 0, wlen);
// 		SDL_MixAudio(stream, aframe->buffer, wlen, SDL_MIX_MAXVOLUME);
// 		pThis->m_sync.SetAudioClock(aframe->dpts);
// 		pThis->m_playEvent->UpdatePlayPosition(aframe->dpts);
// 		HL_PRINT("	[AudioFrame] pts:%lld, dts:%f \r\n", aframe->pts, aframe->dpts);
// 		delete aframe;
// 	}
// 	else
// 	{
// 		SDL_memset(stream, 0, len);
// 	}
}
