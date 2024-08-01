#include "VideoRecorder.h"
#include "Screen/CScreenDXGI.h"
#include "Screen/CScreenGDI.h"

CVideoRecorder::CVideoRecorder()
{

}

CVideoRecorder::~CVideoRecorder()
{

}

bool CVideoRecorder::Init(int type, int width, int height)
{
	m_width = width;
	m_height = height;

	if (type == 1)
		m_pScreen = new CScreenDXGI(type, m_width, m_height);
	else if (type == 2)
		m_pScreen = new CScreenGDI(type, m_width, m_height);
	else if (type == 3)
	{

	}

	m_imageSize = m_width * m_height * 4;
	m_pRgbaBuffer = new uint8_t[m_imageSize]();

	return true;
}

void CVideoRecorder::Start()
{
	m_remux.Open("test.mp4", false, true, m_width, m_height);
	m_remux.Start(this);

	m_bRun = true;
	m_thread = std::thread(&CVideoRecorder::Work, this);
}

void CVideoRecorder::Stop()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();

	m_remux.Close();

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
		m_remux.SendRGBData(m_pRgbaBuffer);
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}

void CVideoRecorder::ProgressValue(int second_time)
{

}
