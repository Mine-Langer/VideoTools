#pragma once
#pragma warning(disable:4819)

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libavutil/file.h>
#include <libavutil/time.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/imgutils.h>
#include <libavutil/avassert.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avstring.h>
#include <libavutil/bprint.h>
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/display.h>
#include <libavutil/fifo.h>
#include <libavutil/hwcontext.h>
#include <libavutil/intreadwrite.h>
//#include <libavutil/libm.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/parseutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/samplefmt.h>
//#include <libavutil/thread.h>
#include <libavutil/threadmessage.h>
#include <libavutil/time.h>
#include <libavutil/timestamp.h>
#include <libavcodec/version.h>
#include <libavformat/avformat.h>
#include <libavutil/macros.h>
#include <libavutil/common.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavutil/cpu.h>
#include <libavutil/eval.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

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

#include "SafeQueue.h"
#include "AVFrame.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

using namespace std;


// 标识录音类型
#define SOUNDCARD  1
#define MICROPHONE 2