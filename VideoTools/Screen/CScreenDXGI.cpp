#include "CScreenDXGI.h"
#include <dxgi1_5.h>
#pragma comment(lib,"d3d11.lib")

CScreenDXGI::CScreenDXGI(int type, int width, int height): CScreenRecoder(type, width, height)
{
	Init();
}

CScreenDXGI::~CScreenDXGI()
{

}

bool CScreenDXGI::GetFrame(uint8_t* buffer, int& nSize, int64_t& timestamp)
{
	if (GetFrame(10))
	{
		timestamp = getCurTimestamp();
		if (CopyFrameData(buffer, nSize))
		{
			DoneWithFrame();
			return true;
		}
	}
	return false;
}

bool CScreenDXGI::GetFrame(int timeout /*= 100*/)
{
	IDXGIResource* DesktopResource = nullptr;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	// Get new frame
	HRESULT hr = m_pDeskDupl->AcquireNextFrame(timeout, &FrameInfo, &DesktopResource);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT)
	{
		return false;
	}

	if (FAILED(hr))
	{
		return false;
	}

	// If still holding old frame, destroy it
	if (m_pAcquireDesktopImage)
	{
		m_pAcquireDesktopImage->Release();
		m_pAcquireDesktopImage = nullptr;
	}

	// QI for IDXGIResource
	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pAcquireDesktopImage));
	DesktopResource->Release();
	DesktopResource = nullptr;
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool CScreenDXGI::Init()
{
	if (!InitDevice())
	{
		return false;
	}
	if (!InitDuplication(0))
	{
		return false;
	}
	return true;
}

void CScreenDXGI::Release()
{
	if (m_pDeskDupl)
	{
		m_pDeskDupl->Release();
		m_pDeskDupl = nullptr;
	}

	if (m_pAcquireDesktopImage)
	{
		m_pAcquireDesktopImage->Release();
		m_pAcquireDesktopImage = nullptr;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
}

bool CScreenDXGI::InitDevice()
{
	HRESULT hr = S_OK;

	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;
	// Create device
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, &m_pDevice, &FeatureLevel, nullptr);
		if (FAILED(hr))
		{
			// Device creation success, no need to loop anymore
			return false;
		}
	}

	return true;
}

bool CScreenDXGI::InitDuplication(int idx)
{
	m_OutputIdx = idx;
	// Take a reference on the device
	m_pDevice->AddRef();

	// Get DXGI device
	IDXGIDevice* DxgiDevice = nullptr;
	HRESULT hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
	if (FAILED(hr))
		return false;

	// Get DXGI adapter
	IDXGIAdapter* DxgiAdapter = nullptr;
	hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	DxgiDevice->Release();
	DxgiDevice = nullptr;
	if (FAILED(hr))
		return false;

	// Get output
	IDXGIOutput* DxgiOutput = nullptr;
	hr = DxgiAdapter->EnumOutputs(m_OutputIdx, &DxgiOutput);
	DxgiAdapter->Release();
	DxgiAdapter = nullptr;
	if (FAILED(hr))
		return false;

	// QI for Output 1
	IDXGIOutput5* DxgiOutput5 = nullptr;
	hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput5), reinterpret_cast<void**>(&DxgiOutput5));
	DxgiOutput->Release();
	DxgiOutput = nullptr;
	if (FAILED(hr))
		return false;

	// Create desktop duplication
	const DXGI_FORMAT formats[] = { DXGI_FORMAT_B8G8R8A8_UNORM };
	hr = DxgiOutput5->DuplicateOutput1(m_pDevice, 0, ARRAYSIZE(formats), formats, &m_pDeskDupl);
	DxgiOutput5->Release();
	DxgiOutput5 = nullptr;
	if (FAILED(hr))
	{
		if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
		{
			// There is already the maximum number of applications using the Desktop Duplication API running, please close one of those applications and then try again.
			return false;
		}

		//切换屏幕创建 Duplication失败时触发的错误
		return false;
	}

	//获取屏幕分辨率
	DXGI_OUTDUPL_DESC OutDupl_Desc;
	m_pDeskDupl->GetDesc(&OutDupl_Desc);

	m_ScreenWidth = OutDupl_Desc.ModeDesc.Width;
	m_ScreenHeight = OutDupl_Desc.ModeDesc.Height;

	return true;
}

bool CScreenDXGI::CopyFrameData(uint8_t* buffer, int& size)
{
	HRESULT hr;
	ID3D11DeviceContext* context;
	m_pDevice->GetImmediateContext(&context);

	D3D11_TEXTURE2D_DESC desc;
	m_pAcquireDesktopImage->GetDesc(&desc);

	ID3D11Texture2D* destImage = nullptr;
	D3D11_TEXTURE2D_DESC desc2;
	desc2.Width = desc.Width;
	desc2.Height = desc.Height;
	desc2.MipLevels = desc.MipLevels;
	desc2.ArraySize = desc.ArraySize;
	desc2.Format = desc.Format;
	desc2.SampleDesc = desc.SampleDesc;
	desc2.Usage = D3D11_USAGE_STAGING;
	desc2.BindFlags = 0;
	desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc2.MiscFlags = 0;
	//不能直接使用m_AquiredDesktopImage的desc描述信息,需要拷贝一个
	hr = m_pDevice->CreateTexture2D(&desc2, nullptr, &destImage);
	if (FAILED(hr)) {
		return false;
	}

	context->CopyResource(destImage, m_pAcquireDesktopImage);//源Texture2D和目的Texture2D需要有相同的多重采样计数和质量时（就是一些属性相同）
	//所以目的纹理对象应该初始化设置，使用m_AcquiredDesktopImage的描述信息来创建纹理（代码是前几行）

	D3D11_MAPPED_SUBRESOURCE desc2_map_res;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	hr = context->Map(destImage, subresource, D3D11_MAP_READ, 0, &desc2_map_res);
	if (FAILED(hr)) {
		return false;
	}
	//size = desc2.Height * desc2_map_res.RowPitch;

	memcpy_s(buffer, size, desc2_map_res.pData, size);

	context->Unmap(destImage, subresource);
	destImage->Release();
	return true;
}

bool CScreenDXGI::DoneWithFrame()
{
	HRESULT hr = m_pDeskDupl->ReleaseFrame();
	if (FAILED(hr))
	{
		return false;
	}

	if (m_pAcquireDesktopImage)
	{
		m_pAcquireDesktopImage->Release();
		m_pAcquireDesktopImage = nullptr;
	}

	return true;
}
