#pragma once
#pragma execution_character_set("utf-8")

extern "C" 
{
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
#include "libsdl/SDL.h"
}


#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib")


#include <windows.h>
#include <errno.h>
#include <limits.h>
#include <tchar.h>
#include <stdlib.h>
#include <cstdio>
#include <time.h>
#include <atomic>
#include <climits>
#include <string>
#include <vector>
#include <queue>
#include <chrono>
#include <cstdint>
#include <thread>
#include <cassert>
#include <cmath>


#ifdef _DEBUG
#define HL_PRINT(...) { char szText[1024]={0}; sprintf_s(szText, 1024, __VA_ARGS__); OutputDebugStringA(szText); }
#else
#define HL_PRINT(...) 
#endif

#define AVSYNC_DYNAMIC_COEFFICIENT 0.0160119  // 动态帧率算法的系数 解方程 (1+x)^6 = 1.1 即
												// 在相差时间(ffmepg时间) 为 6位数的时候，控制
												// 帧率的延时会在标准延时下增加或减少相差时间的
												// (1.1-1)倍

#define AVSYNC_DYNAMIC_THRESHOLD 0.003        // 音视频同步动态帧率进行干预的二者当前时间差的阈值



enum AVType { TAudio, TVideo, TAll };

// 标识录音类型
#define SOUNDCARD  1
#define MICROPHONE 2


#include "SafeQueue.h"
#include "AVFrame.h"
#include "avSync.h"