#pragma once
#include "Common.h"

class CAVFrame
{
public:
	CAVFrame();
	~CAVFrame();

	void Clear();
	void CopyYUV(AVFrame* _data, int _w, int _h);
	void CopyPCM(uint8_t* _buf, int len);

public:
	double dpts;
	double duration;
	int64_t pts;

	int data_channel;
	int linesize[AV_NUM_DATA_POINTERS];
	int data_len[AV_NUM_DATA_POINTERS];
	uint8_t* data[AV_NUM_DATA_POINTERS];
};

