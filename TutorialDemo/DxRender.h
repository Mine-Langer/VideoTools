#pragma once
#include <d3d9.h>
#include <Windows.h>
#include <limits>
#include <string>
#include <mutex>

#pragma comment (lib, "d3d9.lib")

class DxRender
{
public:
	virtual ~DxRender();

	bool Init(HWND hWnd, int nWidth, int nHeight, bool isYuv);

	bool Render(BYTE* pBuf);


private:
	IDirect3D9*				m_pD3D = nullptr;
	IDirect3DDevice9*		m_pDevice = nullptr;
	IDirect3DSurface9*		m_pSurfaceRender = nullptr;
	
	IDirect3DPixelShader9*	m_pMultiPxShader = nullptr;
	IDirect3DVertexBuffer9* m_pQuadVertBuf = nullptr;

	IDirect3DTexture9*		m_pTexY = nullptr;
	IDirect3DTexture9*		m_pTexU = nullptr;
	IDirect3DTexture9*		m_pTexV = nullptr;

	
	RECT m_viewRect;

	int m_nWidth = 320;
	int m_nHeight = 240;
	bool m_bYuv = false;
};

