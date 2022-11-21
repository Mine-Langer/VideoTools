#pragma once
//test
#include "Demultiplexer.h"
//#include "AudioDecoder.h"
//#include "AudioEncoder.h"

class AVTools :public IDemuxEvent
{
public:
	AVTools();
	~AVTools();

	bool run();
	
private:
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;


private:
	CDemultiplexer*	_demux = nullptr;
	//CAudioDecoder	_audioDecoder;
};

