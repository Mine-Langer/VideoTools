#include "player.h"

CPlayer::CPlayer()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

CPlayer::~CPlayer()
{

}

bool CPlayer::Open(const char* szInput)
{
	if (!m_demux.Open(szInput))
		return false;

	if (m_videoDecoder.Open(&m_demux))
	{
		int width = 0; 
		int height = 0; 
		AVPixelFormat pix_fmt;
		m_videoDecoder.GetSrcParameter(width, height, pix_fmt);
	}

	if (m_audioDecoder.Open(&m_demux))
	{
		int sample_rate; 
		AVChannelLayout ch_layout; 
		AVSampleFormat sample_fmt;
		m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
		m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);
		m_dxAudio.Open(ch_layout.nb_channels, sample_rate);

		m_audioSpec.freq = sample_rate;
		m_audioSpec.channels = ch_layout.nb_channels;
		m_audioSpec.samples = 1024;
		m_audioSpec.format = AUDIO_S16SYS;
		m_audioSpec.silence = 0;
		m_audioSpec.userdata = this;
		m_audioSpec.callback = OnAudioCallback;
	}

	return true;
}

void CPlayer::SetView(HWND hWnd, int w, int h)
{
	m_videoDecoder.SetSwsConfig(&m_rect, w, h);

	m_pWindow = SDL_CreateWindowFrom(hWnd);

	m_pRender = SDL_CreateRenderer(m_pWindow, -1, 0);

	m_pTexture = SDL_CreateTexture(m_pRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_rect.w, m_rect.h);

	if (0 > SDL_OpenAudio(&m_audioSpec, nullptr))
	{
		printf("SDL_OpenAudio failed.\n");
		return;
	}
	SDL_PauseAudio(0);
}


void CPlayer::Start()
{
	m_demux.Start(this);
	m_videoDecoder.Start(this);
	m_audioDecoder.Start(this);

	m_bRun = true;
	m_tRender = std::thread(&CPlayer::OnRenderProc, this);
}

void CPlayer::Stop()
{

}

void CPlayer::Release()
{

}


void CPlayer::OnRenderProc()
{
	int64_t delay = 0;
	AVFrame* pFrame = nullptr;
	while (m_bRun)
	{
		if (!m_videoFrameQueue.Pop(pFrame)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
			continue;
		}

		if (pFrame)
		{
			SDL_UpdateYUVTexture(m_pTexture, nullptr, pFrame->data[0], pFrame->linesize[0],
				pFrame->data[1], pFrame->linesize[1], pFrame->data[2], pFrame->linesize[2]);
			SDL_RenderClear(m_pRender);
			SDL_RenderCopy(m_pRender, m_pTexture, nullptr, &m_rect);
			//m_dxVideo.Render(pFrame->data[0], pFrame->data[1], pFrame->data[2]);
			//printf("video frame:[Y:%d, U:%d, V:%d]\n", pFrame->linesize[0], pFrame->linesize[1], pFrame->linesize[2]);

			int64_t _pts = pFrame->pts * m_videoDecoder.Timebase();
			delay = m_avSync.CalcDelay(_pts);
			if (delay > 0)	
				std::this_thread::sleep_for(std::chrono::microseconds(delay));

			SDL_RenderPresent(m_pRender);
			av_frame_free(&pFrame);
			
			m_avSync.SetVideoShowTime();
		}
		else
			m_bRun = false;
	}
}

void CPlayer::OnAudioCallback(void* userdata, Uint8* stream, int len)
{
	CPlayer* pThis = (CPlayer*)userdata;

	AVFrame* pFrame = nullptr;
	if (pThis->m_audioFrameQueue.Pop(pFrame))
	{
		if (!pFrame)
			return;

		int wlen = len < pFrame->linesize[0] ? len : pFrame->linesize[0];
		SDL_memset(stream, 0, wlen);
		SDL_MixAudio(stream, pFrame->data[0], wlen, SDL_MIX_MAXVOLUME);
		double rpts = pFrame->best_effort_timestamp * pThis->m_audioDecoder.Timebase();
		pThis->m_avSync.SetAudioClock(rpts);
		//printf("	[AudioFrame] pts:%lld, dts:%f \r\n", pFrame->pts, pFrame->pts * pThis->m_audioDecoder.timebase());
		av_frame_free(&pFrame);
	}
	else
	{
		SDL_memset(stream, 0, len);
	}
}

bool CPlayer::DemuxPacket(AVPacket* pkt, int type)
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

bool CPlayer::VideoEvent(AVFrame* frame)
{
	m_videoFrameQueue.MaxSizePush(frame, &m_bRun);

	return true;
}

bool CPlayer::AudioEvent(AVFrame* frame)
{
	m_audioFrameQueue.MaxSizePush(frame, &m_bRun);

	return true;
}