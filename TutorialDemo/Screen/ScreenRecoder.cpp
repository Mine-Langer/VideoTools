#include "ScreenRecoder.h"
#include <chrono>

CScreenRecoder::CScreenRecoder()
{

}

CScreenRecoder::~CScreenRecoder()
{

}

int64_t CScreenRecoder::getCurTimestamp()// 获取毫秒级时间戳（13位）
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
