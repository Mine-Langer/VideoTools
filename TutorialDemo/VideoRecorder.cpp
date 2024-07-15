#include "VideoRecorder.h"
#include "Screen/CScreenDXGI.h"
#include "Screen/CScreenGDI.h"

CVideoRecorder::CVideoRecorder()
{

}

CVideoRecorder::~CVideoRecorder()
{

}

bool CVideoRecorder::Init(int type)
{
	if (type == 1)
		m_pScreen = new CScreenDXGI();
	else if (type == 2)
		m_pScreen = new CScreenGDI();
	else if (type == 3)
	{

	}

	m_imageSize = m_width * m_height * 4;
	m_pRgbaBuffer = new uint8_t[m_imageSize]();

	return true;
}

void CVideoRecorder::Start()
{
	m_bRun = true;
	m_thread = std::thread(&CVideoRecorder::Work, this);
}

void CVideoRecorder::Stop()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_pScreen)
	{
		delete m_pScreen;
		m_pScreen = nullptr;
	}
}

void CVideoRecorder::Work()
{
	int ret = 0;
	int64_t timestamp = 0;

	while (m_bRun)
	{
		m_pScreen->GetFrame(m_pRgbaBuffer, m_imageSize, timestamp);
	}
}
