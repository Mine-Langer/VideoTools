#pragma once
#include "Remultiplexer.h"

class CVideoRecorder : public ITranscodeProgress
{
public:
	CVideoRecorder();
	~CVideoRecorder();

	bool Init(int type, int width, int height);

	void Start();

	void Stop();

protected:
	void Work();

	virtual void ProgressValue(int second_time) override;

private:
	int m_width;
	int m_height;
	int m_imageSize = 0;
	uint8_t* m_pRgbaBuffer = nullptr;
	class CScreenRecoder* m_pScreen;	// Â¼ÖÆÆÁÄ»½Ó¿Ú

	CRemultiplexer	m_remux;

	bool m_bRun = false;
	std::thread m_thread;
};

