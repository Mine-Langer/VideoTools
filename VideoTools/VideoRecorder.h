#pragma once
#include "Common.h"

class CVideoRecorder
{
public:
	CVideoRecorder();
	~CVideoRecorder();

	bool Init(int type);

	void Start();

	void Stop();

protected:
	void Work();

private:
	int m_width;
	int m_height;
	int m_imageSize = 0;
	uint8_t* m_pRgbaBuffer = nullptr;
	class CScreenRecoder* m_pScreen;

	bool m_bRun = false;
	std::thread m_thread;
};

