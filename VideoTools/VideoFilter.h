#pragma once
#include "Common.h"

class CVideoFilter
{
public:
	CVideoFilter();
	~CVideoFilter();

	bool Init(int nWidth, int nHeight, AVPixelFormat pix_fmt, AVRational sampleRatio, AVRational timebase);

	void SetFilter(const char* szFilter);

	AVFrame* Convert(AVFrame* srcFrame, bool& bRet);

	//  初始化水印图像
	bool InitWaterMask();

protected:
	void Release();

	// 辅助函数
	AVFormatContext* open_input_from_memory(uint8_t* buffer, size_t buffer_size);
	// 读取回调函数
	static int read_packet(void* opaque, uint8_t* buf, int buf_size);
	// 关闭回调函数
	static int64_t seek_packet(void* opaque, int64_t offset, int whence);

	// 随机生成水印位置
	void get_random_position(int video_width, int video_height, int watermark_width, int watermark_height, int& x, int& y);

private:
	AVFilterGraph* m_filterGraph = nullptr;
	AVFilterContext* m_bufferSinkCtx = nullptr;
	AVFilterContext* m_bufferSrcCtx = nullptr;
	AVFilterContext* m_watermarkSrcCtx = nullptr;

	AVFrame* m_pWaterMaskFrame = nullptr;

	int m_VideoWidth, m_VideoHeight;
	int m_WaterWidth, m_WaterHeight;

	bool m_bRandomPosition = true;

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