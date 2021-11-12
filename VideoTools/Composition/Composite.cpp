#include "Composite.h"

Composite::Composite()
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

Composite::~Composite()
{

}

bool Composite::OpenImage(const char* szFile)
{
	if (!m_videoDecoder.Open(szFile))
		return false;

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

	m_audioSpec.freq = sample_rate;
	m_audioSpec.format = AUDIO_S16SYS;
	m_audioSpec.channels = av_get_channel_layout_nb_channels(ch_layout);
	m_audioSpec.silence = 0;
	m_audioSpec.samples = nb_sample;
	m_audioSpec.userdata = this;
	m_audioSpec.callback = OnSDLAudioFunction;

	if (0 > SDL_OpenAudio(&m_audioSpec, nullptr))
		return false;

	SDL_PauseAudio(0);

	return true;
}

void Composite::Start()
{
	
}

void Composite::Close()
{
	while (!m_videoQueue.empty())
	{
		AVFrame* vdata = m_videoQueue.front();
		m_videoQueue.pop();
		av_frame_free(&vdata);
	}

	m_videoDecoder.Release();
}

void Composite::Play()
{
	m_playThread = std::thread(&Composite::OnPlayFunction, this);
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

	return true;
}

bool Composite::VideoEvent(AVFrame* frame)
{
	AVFrame* vdata = av_frame_clone(frame);
	m_videoQueue.push(vdata);
	return true;
}

bool Composite::AudioEvent(AVFrame* frame)
{
	m_audioQueue.push(frame);
	return true;
}

void Composite::OnSDLAudioFunction(void* userdata, Uint8* stream, int len)
{
	Composite* pThis = (Composite*)userdata;
	
	if (pThis->m_audioQueue.empty())
	{
		SDL_memset(stream, 0, len);
	}
	else
	{
		AVFrame* frame = pThis->m_audioQueue.front();
		pThis->m_audioQueue.pop();
		int wLen = len < frame->linesize[0] ? len : frame->linesize[0];
		SDL_memset(stream, 0, len);
		SDL_MixAudio(stream, frame->data[0], wLen, SDL_MIX_MAXVOLUME);

		av_frame_free(&frame);
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
			m_videoQueue.front();
		}
	}
}
