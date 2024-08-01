#pragma once
#include <string>

class CScreenRecoder
{
public:
	CScreenRecoder(int type, int width, int height);
	~CScreenRecoder();

	virtual bool GetFrame(uint8_t* buffer, int& nSize, int64_t& timestamp) = 0;

protected:
	int64_t getCurTimestamp();

protected:
	int m_type;
	int m_width;
	int m_height;
	int m_factWidth;
	int m_factHeight;
};

