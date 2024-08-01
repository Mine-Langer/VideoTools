#include "ScreenRecoder.h"
#include <chrono>

CScreenRecoder::CScreenRecoder(int type, int width, int height): m_type(type), m_width(width), m_height(height)
{

}

CScreenRecoder::~CScreenRecoder()
{

}

int64_t CScreenRecoder::getCurTimestamp()// ��ȡ���뼶ʱ�����13λ��
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
