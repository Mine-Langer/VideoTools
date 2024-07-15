#include "CScreenGDI.h"
#pragma comment (lib, "gdiplus.lib")


CScreenGDI::CScreenGDI()
{
	Init();
}

CScreenGDI::~CScreenGDI()
{
	Release();
}

bool CScreenGDI::GetFrame(uint8_t* buffer, int& nSize, int64_t& timestamp)
{
	timestamp = getCurTimestamp();

	BitBlt(m_hCaptureDC, 0, 0, m_width, m_height, m_hDC, 0, 0, SRCCOPY);

	GetDIBits(m_hCaptureDC, m_hCaptureBitmap, 0, m_height, m_pRGB, &m_BmpInfo, DIB_RGB_COLORS);

	memcpy(buffer, m_pRGB, m_RgbSize);

	nSize = m_RgbSize;

	return true;
}

bool CScreenGDI::Init()
{
	m_hWinSta = GetProcessWindowStation();
	if (!m_hWinSta)
		return false;

	m_hWnd = GetDesktopWindow();
	m_hDC = GetDC(m_hWnd);

	// 初始化GDI
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if (Gdiplus::GdiplusStartup(&m_GdiplusToken, &gdiplusStartupInput, nullptr) != Gdiplus::Ok)
		return false;

	m_hCaptureDC = CreateCompatibleDC(m_hDC);
	m_hCaptureBitmap = CreateCompatibleBitmap(m_hDC, m_width, m_height);
	SelectObject(m_hCaptureDC, m_hCaptureBitmap);

	m_BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_BmpInfo.bmiHeader.biWidth = m_width;
	m_BmpInfo.bmiHeader.biHeight = -m_height;
	m_BmpInfo.bmiHeader.biPlanes = 1;
	m_BmpInfo.bmiHeader.biBitCount = 24;
	m_BmpInfo.bmiHeader.biCompression = BI_RGB;
	m_BmpInfo.bmiHeader.biSizeImage = 0;
	m_BmpInfo.bmiHeader.biXPelsPerMeter = 0;
	m_BmpInfo.bmiHeader.biYPelsPerMeter = 0;
	m_BmpInfo.bmiHeader.biClrUsed = 0;
	m_BmpInfo.bmiHeader.biClrImportant = 0;

	m_RgbSize = m_width * m_height * 3;//24位图像大小
	m_pRGB = (PRGBTRIPLE)malloc(m_RgbSize);

	return true;
}

void CScreenGDI::Release()
{
	if (m_pRGB) 
	{
		free(m_pRGB);
		m_pRGB = nullptr;
	}

	if (m_hCaptureBitmap) {
		DeleteObject(m_hCaptureBitmap);
		m_hCaptureBitmap = nullptr;
	}

	if (m_hCaptureDC) {
		DeleteDC(m_hCaptureDC);
		m_hCaptureDC = nullptr;
	}

	//卸载GDI
	Gdiplus::GdiplusShutdown(m_GdiplusToken);

	if (m_hWnd) {
		ReleaseDC(m_hWnd, m_hDC);
		m_hWnd = nullptr;
	}
}
