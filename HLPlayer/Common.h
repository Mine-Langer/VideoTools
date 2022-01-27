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
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "postproc.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib")

#include <thread>
#include <iostream>
#include <string>

#include "SafeQueue.h"
#include "avAsync.h"

enum eAVStatus { ePlay, ePause, eStop };