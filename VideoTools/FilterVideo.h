#pragma once
#include "Common.h"

class CFilterVideo
{
public:
	CFilterVideo();
	~CFilterVideo();

	bool Init(int nWidth, int nHeight, AVPixelFormat pix_fmt, AVRational sampleRatio, AVRational timebase);

	void SetFilter(const char* szFilter);

	AVFrame* Convert(AVFrame* srcFrame);

private:
	AVFilterGraph* m_filterGraph = nullptr;
	AVFilterContext* m_bufferSinkCtx = nullptr;
	AVFilterContext* m_bufferSrcCtx = nullptr;

	std::string m_szFilter;
};

class CDrawText
{
public:
	CDrawText();
	~CDrawText();

	bool StartDrawText(const char* pFileA, const char* pFileOut, int x, int y, int iFontSize, std::string strText);

	bool WaitFinish();

private:
	bool OpenFile(const char* pFile);

	bool OpenOutput(const char pFileOut);

	bool InitFilter(const char* filter_desc);


};