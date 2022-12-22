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
		AVRational sampleRatio, timebase;
		m_videoDecoder.GetSrcParameter(width, height, pix_fmt);
		m_videoDecoder.GetSrcRational(sampleRatio, timebase);
		std::string strFontFile;
		strFontFile = "D:\\github.com\\VideoTools\\bin\\x64\\hanyiwenboguliw.ttf";
		char szFilterDesc[512] = { 0 };
		_snprintf_s(szFilterDesc, sizeof(szFilterDesc),
			"drawtext=fontfile=\'%s\':fontcolor=blue:fontsize=36:text=\'libing044@gmail  --Langer\':x=100:y=50", strFontFile.c_str());

		m_filter.SetFilter(szFilterDesc); // "movie=logo.png[wm];[in][wm]overlay=5:5[out]" "drawtext=fontsize=60:text='%{localtime\:%Y\-%m\-%d %H-%M-%S}':fontcolor=green:box=1:boxcolor=yellow"
		m_filter.Init(width, height, pix_fmt, sampleRatio, timebase);
	}

	if (m_audioDecoder.Open(&m_demux))
	{
		int sample_rate; 
		AVChannelLayout ch_layout; 
		AVSampleFormat sample_fmt;
		m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
		int samples = m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);
		m_dxAudio.Open(ch_layout.nb_channels, sample_rate);

		// ÉèÖÃSDL_AudioSpecÊôÐÔ
		SetAudioSpec(sample_rate, ch_layout, samples);
	}

	return true;
}

void CPlayer::SetView(HWND hWnd, int w, int h)
{
	m_videoDecoder.SetSwsConfig(&m_rect, w, h);

	m_pWindow = SDL_CreateWindowFrom(hWnd);

	m_pRender = SDL_CreateRenderer(m_pWindow, -1, 0);

	m_pTexture = SDL_CreateTexture(m_pRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_rect.w, m_rect.h);

	//PlayAudio();
}

void CPlayer::SetAudioSpec(int sample_rate, AVChannelLayout ch_layout, int samples)
{
	m_audioSpec.freq = sample_rate;
	m_audioSpec.channels = ch_layout.nb_channels;
	m_audioSpec.samples = samples;
	m_audioSpec.format = AUDIO_S16SYS;
	m_audioSpec.silence = 0;
	m_audioSpec.userdata = this;
	m_audioSpec.callback = OnAudioCallback;
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

void CPlayer::Pause()
{
	m_pause = !m_pause;
	SDL_PauseAudio(!m_pause);
}

void CPlayer::Seek(uint64_t pts_time)
{
	m_demux.SetPosition(pts_time);
}

void CPlayer::Release()
{
	SDL_PauseAudio(1);
	if (m_pWindow) {
		SDL_DestroyWindow(m_pWindow);
		m_pWindow = nullptr;
	}

	if (m_pRender) {
		SDL_DestroyRenderer(m_pRender);
		m_pRender = nullptr;
	}

	if (m_pTexture) {
		SDL_DestroyTexture(m_pTexture);
		m_pTexture = nullptr;
	}
}


void CPlayer::SendAudioFrame(AVFrame* frame)
{
	m_audioFrameQueue.MaxSizePush(frame, &m_bRun);
}

void CPlayer::SendVideoFrame(AVFrame* frame)
{
	m_videoFrameQueue.MaxSizePush(frame, &m_bRun);
}

void CPlayer::PlayAudio()
{
	if (0 > SDL_OpenAudio(&m_audioSpec, nullptr))
	{
		printf("SDL_OpenAudio failed.\n");
		return;
	}
	SDL_PauseAudio(0);
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

			double _pts = pFrame->best_effort_timestamp * m_videoDecoder.Timebase();
			delay = m_avSync.CalcDelay(_pts);
			if (delay > 0)	
				std::this_thread::sleep_for(std::chrono::microseconds(delay));

			//printf("video pts:%lld, timebase:%f, time:%f \n", pFrame->best_effort_timestamp, m_videoDecoder.Timebase(), _pts);

			SDL_RenderPresent(m_pRender);
			
			if (!m_pause)
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

	if (pThis->m_pause) {
		SDL_memset(stream, 0, len);
		return;
	}

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
		printf("audio pts:%lld, timebase:%f time:%f \r\n", pFrame->best_effort_timestamp, pThis->m_audioDecoder.Timebase(), rpts);
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

void CPlayer::CleanPacket()
{
	AVFrame* frame = nullptr;
	m_audioDecoder.Clear();
	m_videoDecoder.Clear();
	while (!m_videoFrameQueue.Empty())
	{
		if (m_videoFrameQueue.Pop(frame))
			av_frame_free(&frame);
	}

	while (!m_audioFrameQueue.Empty())
	{
		if (m_audioFrameQueue.Pop(frame))
			av_frame_free(&frame);
	}
}

bool CPlayer::VideoEvent(AVFrame* frame)
{
	AVFrame* filterFrame = m_filter.Convert(frame);
	m_videoFrameQueue.MaxSizePush(filterFrame, &m_bRun);
	av_frame_free(&frame);

	return true;
}

bool CPlayer::AudioEvent(AVFrame* frame)
{
	m_audioFrameQueue.MaxSizePush(frame, &m_bRun);

	return true;
}