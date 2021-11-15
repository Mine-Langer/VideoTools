#pragma once

extern "C" 
{
#include "SDL.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/pixdesc.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libavutil/file.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavutil/fifo.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#pragma comment (lib, "SDL2.lib")
//#pragma comment (lib, "SDL2main.lib")
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "postproc.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib")

#include "SafeQueue.h"

#include <thread>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <tchar.h>

#ifdef _DEBUG
#include <windows.h>
#define HL_PRINT(...) { char szText[1024]={0}; sprintf_s(szText, 1024, __VA_ARGS__); OutputDebugStringA(szText); }
#else
#define HL_PRINT(...) 
#endif

#define AVSYNC_DYNAMIC_COEFFICIENT 0.0160119  // ��̬֡���㷨��ϵ�� �ⷽ�� (1+x)^6 = 1.1 ��
												// �����ʱ��(ffmepgʱ��) Ϊ 6λ����ʱ�򣬿���
												// ֡�ʵ���ʱ���ڱ�׼��ʱ�����ӻ�������ʱ���
												// (1.1-1)��

#define AVSYNC_DYNAMIC_THRESHOLD 0.003        // ����Ƶͬ����̬֡�ʽ��и�Ԥ�Ķ��ߵ�ǰʱ������ֵ


enum RecordState { NotStarted, Started, Paused, Stopped, Unknowned };

class IVideoEvent
{
public:
	virtual bool VideoEvent(AVFrame* frame) = 0;
};

class IAudioEvent
{
public:
	virtual bool AudioEvent(AVFrame* frame) = 0;
};

class IDemuxEvent
{
public:
	virtual bool DemuxPacket(AVPacket* pkt, int type) = 0;
};