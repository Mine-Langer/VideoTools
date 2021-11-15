#pragma once
#include "Common.h"

class CFilterVideo
{
public:
	CFilterVideo();
	~CFilterVideo();

	bool Init();

private:
	AVFilterGraph* m_filterGraph = nullptr;
	AVFilterContext* m_bufferSinkCtx = nullptr;
	AVFilterContext* m_bufferSrcCtx = nullptr;
};

