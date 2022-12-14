#pragma once
#include "../Demultiplexer.h"
#include "../AudioDecoder.h"
#include "../VideoEncoder.h"
#include "../AudioEncoder.h"
#include "../FilterVideo.h"
#include "../player.h"
#include "AVTextureBar.h"

#define E_Play 0x1
#define E_Save 0x2
class Composite :public IDecoderEvent, public IDemuxEvent
{
public:
	Composite();
	~Composite();

	void Start();

	void Close();

	void Play(std::vector<ItemElem>& vecImage, std::vector<ItemElem>& vecMusic); // 播放

	// 设置播放时的显示参数
	bool InitWnd(HWND pWnd, int width, int height);

	// 保存文件
	bool SaveFile(const char* szOutput, std::vector<ItemElem>& vecImage, std::vector<ItemElem>& vecMusic);

private:

	virtual bool VideoEvent(AVFrame* frame) override;
	virtual bool AudioEvent(AVFrame* frame) override;
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;
	virtual void CleanPacket()override;

private:
	bool OpenAudio(std::vector<ItemElem>& vecAudio);
	bool OpenImage(std::vector<ItemElem>& vecImage);

	bool GetAudioImage(const char* filename);

	bool InitAudioEnc(enum AVCodecID codec_id);

private:
	void OnPlayFunction(); // 
	void OnSaveFunction(); // 保存文件
	void OnDemuxFunction();// 解码线程

private:
	CPlayer	m_player;
	CDemultiplexer m_demuxImage;
	CDemultiplexer m_demuxMusic;
	CVideoDecoder m_videoDecoder;
	CAudioDecoder m_audioDecoder;
	CVideoEncoder m_videoEncoder;
	CAudioEncoder m_audioEncoder;

	SafeQueue<AVFrame*> m_videoQueue;
	SafeQueue<AVFrame*> m_audioQueue;

	SafeQueue<AVPacket*> m_videoPktQueue;
	SafeQueue<AVPacket*> m_audioPktQueue;

	AVFormatContext* m_pOutFormatCtx = nullptr;
	CFilterVideo m_filter;

	int m_nType = 0;

	HWND m_hWndView;
	int m_videoWidth = 0;
	int m_videoHeight = 0;

	// 480p=720×480  720p=1280×720 1080p=1920×1080 4k=2160p=3840×2160
	int m_outputWidth = 1280;
	int m_outputHeight = 720;

	int m_bitRate = 4000000;
	int m_frameRate = 25;
	int m_audioFrameSize = 0;

	std::thread m_playThread;
	std::thread m_saveThread;
	std::thread m_demuxThread;
	AVState m_state = NotStarted;
	int m_type = 0; // 0: 预览播放  1：保存文件
	std::vector<ItemElem> m_vecImage;
	std::vector<ItemElem> m_vecMusic;
};

class CImageFrame
{
public:
	~CImageFrame();

	bool Open(const char* szfile);

	AVFrame* ImageFrame();

private:
	void Release();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	AVFrame* m_pFrameData = nullptr;
};


class CAudioFrame
{
public:
	~CAudioFrame();

	void SetRange(int begin, int end);

	bool Open(const char* szfile, int begin = 0, int end = 0);

	AVFrame* AudioFrame(bool& bstatus);

private:
	void Release();

	void OnRun();

private:
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	SwrContext* m_swr_ctx = nullptr;

	int64_t m_begin_pts = 0, m_end_pts = 0;
	int m_audio_idx = -1;

	int m_swr_sample_rate = 0;
	AVChannelLayout m_swr_ch_layout;
	AVSampleFormat m_swr_sample_fmt;

	SafeQueue<AVFrame*> m_frameQueueData;
	std::thread m_tRead;
	bool m_bRun = false;

	double m_timebase = 0.0;
};