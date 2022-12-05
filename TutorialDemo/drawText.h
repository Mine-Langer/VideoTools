#pragma once
#include "Common.h"

class CDrawText
{
public:
	CDrawText();
	~CDrawText();
public:
	int StartDrawText(const char* pFileA, const char* pFileOut, int x, int y, int iFontSize, std::string strText);
	int WaitFinish();
private:
	int OpenFileA(const char* pFileA);
	int OpenOutPut(const char* pFileOut);
	int InitFilter(const char* filter_desc);
private:
	static DWORD WINAPI VideoAReadProc(LPVOID lpParam);
	void VideoARead();

	static DWORD WINAPI VideoDrawTextProc(LPVOID lpParam);
	void VideoDrawText();

private:
	AVFormatContext* m_pFormatCtx_FileA = NULL;

	AVCodecContext* m_pReadCodecCtx_VideoA = NULL;
	AVCodec* m_pReadCodec_VideoA = NULL;


	AVCodecContext* m_pCodecEncodeCtx_Video = NULL;
	AVFormatContext* m_pFormatCtx_Out = NULL;

	AVFifo* m_pVideoAFifo = NULL;


	int m_iVideoWidth = 1920;
	int m_iVideoHeight = 1080;
	int m_iYuv420FrameSize = 0;

private:

	AVFilterGraph* m_pFilterGraph = NULL;
	AVFilterContext* m_pFilterCtxSrcVideoA = NULL;
	AVFilterContext* m_pFilterCtxSink = NULL;

private:

	CRITICAL_SECTION m_csVideoASection;
	HANDLE m_hVideoAReadThread = NULL;
	HANDLE m_hVideoDrawTextThread = NULL;
};

