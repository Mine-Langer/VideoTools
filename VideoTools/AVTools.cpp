#include "AVTools.h"

AVTools::AVTools()
{
    _demux = new CDemultiplexer();
}

AVTools::~AVTools()
{

}

bool AVTools::run()
{
    if (!_demux->Open("D:/documents/OneDrive/video/QQÊÓÆµ¸ãÇ®.mp4"))
        return false;

    _demux->Start(this);

//    if (!_audioDecoder.Open(_demux))
//        return false;
//    
//    int sample_rate;
//    AVChannelLayout ch_layout;
//    AVSampleFormat sample_fmt;
//    _audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
//
//    _audioDecoder.SetSwrContext(ch_layout, sample_fmt, sample_rate);
	
    return true;
}

bool AVTools::DemuxPacket(AVPacket* pkt, int type)
{
    if (type == AVMEDIA_TYPE_AUDIO) {

    }
    else if (type == AVMEDIA_TYPE_VIDEO) {

    }
    return false;
}
