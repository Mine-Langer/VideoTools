#pragma once
#include "ScreenRecoder.h"
#include <d3d11.h>
#include <dxgi1_2.h>

class CScreenDXGI : public CScreenRecoder
{
public:
	CScreenDXGI(int type, int width, int height);
	~CScreenDXGI();


protected:
	virtual bool GetFrame(uint8_t* buffer, int& nSize, int64_t& timestamp) override;

	bool Init();

	void Release();

	bool InitDevice();
	bool InitDuplication(int idx);
	bool CopyFrameData(uint8_t* buffer, int& size);
	bool DoneWithFrame();
	bool GetFrame(int timeout = 100); //millisecond

private:
	ID3D11Device* m_pDevice = nullptr;
	IDXGIOutputDuplication* m_pDeskDupl;
	ID3D11Texture2D* m_pAcquireDesktopImage;
	UINT m_OutputIdx = 0;
	int m_ScreenWidth = 0;
	int m_ScreenHeight = 0;
};

