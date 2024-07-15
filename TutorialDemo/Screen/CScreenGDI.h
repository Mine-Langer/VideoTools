#pragma once
#include "ScreenRecoder.h"
#include <windows.h>
#include <gdiplus.h>



class CScreenGDI : public CScreenRecoder
{
public:
	CScreenGDI();
	~CScreenGDI();


protected:
	virtual bool GetFrame(uint8_t* buffer, int& nSize, int64_t& timestamp) override;

	bool Init();

	void Release();

private:
	HWINSTA		m_hWinSta = nullptr;
	HWND		m_hWnd = nullptr;
	HDC			m_hDC = nullptr;
	ULONG_PTR	m_GdiplusToken = 0;

	HDC			m_hCaptureDC;
	HBITMAP		m_hCaptureBitmap;

	BITMAPINFO	m_BmpInfo;
	int			m_RgbSize;
	PRGBTRIPLE	m_pRGB = nullptr;
};

