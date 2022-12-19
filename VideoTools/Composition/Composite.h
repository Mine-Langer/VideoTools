#pragma once
#include "../Demultiplexer.h"
#include "../Remultiplexer.h"
#include "../AudioDecoder.h"
#include "../VideoEncoder.h"
#include "../AudioEncoder.h"
#include "../FilterVideo.h"
#include "../player.h"
#include "AVTextureBar.h"

#define E_Play 0x1
#define E_Save 0x2
class Composite :public IDemuxEvent, public IDecoderEvent, public IRemuxEvent
{
public:
	Composite();
	~Composite();

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
	virtual void CleanPacket() override;

	virtual void RemuxEvent(int nType) override;

private:
	void OpenAudio(std::vector<ItemElem> vecAudio);
	void OpenImage(std::vector<ItemElem> vecImage);

	bool InitAudioEnc(enum AVCodecID codec_id);

private:
	void OnPlayFunction(); // 

	void OnDemuxAFunction();// 音频解码线程
	void OnDemuxVFunction();// 图像解码线程

private:
	CPlayer	m_player;
	CDemultiplexer m_demuxImage;
	CDemultiplexer m_demuxMusic;
	CVideoDecoder m_videoDecoder;
	CAudioDecoder m_audioDecoder;

	CVideoEncoder m_videoEncoder;
	CAudioEncoder m_audioEncoder;

	CRemultiplexer m_remux;

	//SafeQueue<AVFrame*> m_videoQueue;
	//SafeQueue<AVFrame*> m_audioQueue;

	SafeQueue<AVPacket*> m_videoPktQueue;
	SafeQueue<AVPacket*> m_audioPktQueue;

	AVFormatContext* m_pOutFormatCtx = nullptr;
	CFilterVideo m_filter;

	int m_nType = 0;

	bool m_bRun = false;

	HWND m_hWndView;
	int m_videoWidth = 0;
	int m_videoHeight = 0;

	// 480p=720×480  720p=1280×720 1080p=1920×1080 4k=2160p=3840×2160
	int m_outputWidth = 1280;
	int m_outputHeight = 720;
	AVChannelLayout m_out_ch_layout;
	AVSampleFormat m_out_sample_fmt;
	int m_out_sample_rate;

	int m_out_bit_rate = 4000000;
	int m_out_frame_rate = 25;
	int m_audioFrameSize = 0;

	std::thread m_playThread;
	std::thread m_ta_demux;
	std::thread m_tv_demux;

	AVState m_state = NotStarted;
	int m_type = 0; // 0: 预览播放  1：保存文件

};
