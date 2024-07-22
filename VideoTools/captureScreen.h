#pragma once
#include "Common.h"

class CaptureScreen
{
public:
	CaptureScreen();
	~CaptureScreen();

	bool Init(int x, int y, int w, int h);

	uint8_t* CaptureImage();

	uint8_t* YUVBuffer();

private:

	void* CaptureScreenFrame();

	HCURSOR FetchCursorHandle();

	uint8_t clip_value(uint8_t x);
	bool ConvertToYUV420(uint8_t* rgbBuf);

private:
	int m_width = 0;
	int m_height = 0;
	int m_x = 0;
	int m_y = 0;

	HDC m_hScreenDC = nullptr;
	HDC m_hMemDC = nullptr;

	PRGBTRIPLE m_hdib = nullptr;
	BITMAPINFO m_pbi = { 0 };
	HBITMAP m_hbmp = nullptr;
	HCURSOR m_hSaveCursor = nullptr;

	uint8_t* m_pYUVBuffer = nullptr;
};
