#include "DxVideoRender.h"

DxVideoRender::DxVideoRender()
{
}

DxVideoRender::~DxVideoRender()
{
}

bool DxVideoRender::Init(HWND hWnd, int nWidth, int nHeight, bool isYuv)
{
	HRESULT hr;
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D)
		return false;

	D3DPRESENT_PARAMETERS d3dpp = { 0 };
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	GetClientRect(hWnd, &m_viewRect);

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_bYuv = isYuv;

	if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pDevice)))
		return false;

	if (isYuv) {
		if (FAILED(hr = m_pDevice->CreateOffscreenPlainSurface(m_nWidth, m_nHeight, (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'), D3DPOOL_DEFAULT, &m_pSurfaceRender, nullptr)))
		{
			printf("create SurfaceRender failed.\n");
			return false;
		}
	}
	else {
		if (FAILED(m_pDevice->CreateOffscreenPlainSurface(m_nWidth, m_nHeight, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pSurfaceRender, nullptr)))
			return false;
	}

	return true;
}

bool DxVideoRender::Render(BYTE* pBufY, BYTE* pBufU, BYTE* pBufV)
{
	if (!m_pSurfaceRender)
		return false;

	D3DLOCKED_RECT dxRect = { 0 };
	if (FAILED(m_pSurfaceRender->LockRect(&dxRect, NULL, D3DLOCK_DONOTWAIT)))
		return false;

	BYTE* pDest = (BYTE*)dxRect.pBits;
	int stride = dxRect.Pitch;

	if (m_bYuv)
	{
		for (int i = 0; i < m_nHeight; i++)
			memcpy(pDest + i * stride, pBufY, m_nWidth);
		for (int i = 0; i < m_nHeight / 2; i++)
			memcpy(pDest + stride * m_nHeight + i * stride / 2, pBufU, m_nWidth / 2);
		for (int i = 0; i < m_nHeight / 2; i++)
			memcpy(pDest + stride * m_nHeight + stride * m_nHeight / 4 + i * stride / 2, pBufV, m_nWidth / 2);
	}
//	else
//	{
//		int pixelSize = m_nWidth * 4;
//		for (int i = 0; i < m_nHeight; i++) {
//			memcpy(pDest, pSrc, pixelSize);
//			pDest += stride;
//			pDest += stride;
//			pSrc += pixelSize;
//		}
//	}

	if (FAILED(m_pSurfaceRender->UnlockRect()))
		return false;

	m_pDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_pDevice->BeginScene();

	IDirect3DSurface9* pBackBuf = nullptr;
	m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuf);
	m_pDevice->StretchRect(m_pSurfaceRender, nullptr, pBackBuf, &m_viewRect, D3DTEXF_LINEAR);
	m_pDevice->EndScene();

	m_pDevice->Present(nullptr, nullptr, nullptr, nullptr);
	pBackBuf->Release();

	return true;
}
