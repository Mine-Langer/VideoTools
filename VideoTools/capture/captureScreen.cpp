#include "captureScreen.h"

CaptureScreen::CaptureScreen()
{
	FetchCursorHandle();
}

CaptureScreen::~CaptureScreen()
{
	DeleteObject(m_hbmp);
	if (m_hdib)
	{
		free(m_hdib);
		m_hdib = nullptr;
	}
	if (m_hScreenDC)
		::ReleaseDC(nullptr, m_hScreenDC);
	if (m_hMemDC)
		DeleteDC(m_hMemDC);
	if (m_hbmp)
		DeleteObject(m_hbmp);
}

bool CaptureScreen::Init(int x, int y, int w, int h)
{
	m_hScreenDC = ::GetDC(GetDesktopWindow());
	if (!m_hScreenDC)
		return false;

	m_x = x;
	m_y = y;
	m_width = w;
	m_height = h;

	m_hMemDC = ::CreateCompatibleDC(m_hScreenDC);

	if (!m_hdib)
		m_hdib = (PRGBTRIPLE)malloc(m_width * m_height * 3); // 24位图像大小

	// 位图头结构
	m_pbi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pbi.bmiHeader.biWidth = m_width;
	m_pbi.bmiHeader.biHeight = m_height;
	m_pbi.bmiHeader.biPlanes = 1;
	m_pbi.bmiHeader.biBitCount = 24;
	m_pbi.bmiHeader.biCompression = BI_RGB;

	m_hbmp = CreateCompatibleBitmap(m_hScreenDC, m_width, m_height);
	SelectObject(m_hMemDC, m_hbmp);

	// 设置YUV
	unsigned int frameSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, m_width, m_height, 1);
	m_pYUVBuffer = new uint8_t[frameSize];
	if (!m_pYUVBuffer)
		return false;

	return true;
}

uint8_t* CaptureScreen::CaptureImage()
{
	void* alpbi = CaptureScreenFrame();
	ConvertToYUV420((uint8_t*)alpbi);
	return (uint8_t*)alpbi;
}


uint8_t* CaptureScreen::YUVBuffer()
{
	return m_pYUVBuffer;
}

void* CaptureScreen::CaptureScreenFrame()
{
	if (m_hbmp == nullptr || m_hMemDC == nullptr || m_hScreenDC == nullptr)
		return nullptr;

	BitBlt(m_hMemDC, 0, 0, m_width, m_width, m_hScreenDC, m_x, m_y, SRCCOPY);

	// 绘制 鼠标指针
	{
		POINT xPt;
		GetCursorPos(&xPt);
		HCURSOR hCur = FetchCursorHandle();
		xPt.x -= m_x;
		xPt.y -= m_y;

		ICONINFO iconInfo;
		if (GetIconInfo(hCur, &iconInfo))
		{
			xPt.x -= iconInfo.xHotspot;
			xPt.y -= iconInfo.yHotspot;

			if (iconInfo.hbmMask)
				DeleteObject(iconInfo.hbmMask);
			if (iconInfo.hbmColor)
				DeleteObject(iconInfo.hbmColor);
		}

		::DrawIcon(m_hMemDC, xPt.x, xPt.y, hCur);
	}
	
	GetDIBits(m_hMemDC, m_hbmp, 0, m_height, m_hdib, &m_pbi, DIB_RGB_COLORS);
	return m_hdib;
}

HCURSOR CaptureScreen::FetchCursorHandle()
{
	if (!m_hSaveCursor)
	{
		m_hSaveCursor = GetCursor();
	}
	return m_hSaveCursor;
}

uint8_t CaptureScreen::clip_value(uint8_t x)
{
	const int max = 255, min = 0;
	if (x > max)
		x = max;
	else if (x < min)
		x = min;
	return x;
}

bool CaptureScreen::ConvertToYUV420(uint8_t* rgbBuf)
{
	uint8_t *ptrY, *ptrU, *ptrV, *ptrRGB;
	uint8_t Y, U, V, R, G, B;
	memset(m_pYUVBuffer, 0, m_width*m_height*3/2);
	
	ptrY = m_pYUVBuffer;
	ptrU = m_pYUVBuffer + m_width * m_height;
	ptrV = ptrU + (m_width * m_height / 4);

	for (int c=m_height-1; c>=0; c--)
	{
		ptrRGB = rgbBuf + m_width * c * 3;
		for (int row = 0; row < m_width; row++)
		{
			B = *(ptrRGB++);
			G = *(ptrRGB++);
			R = *(ptrRGB++);

			Y = (uint8_t)((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
			U = (uint8_t)((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
			V = (uint8_t)((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;
			*(ptrY++) = clip_value(Y);
			if (c % 2 == 0 && row % 2 == 0)
				*(ptrU++) = clip_value(U);
			else if (row % 2 == 0)
				*(ptrV++) = clip_value(V);
		}
	}
	return true;
}
