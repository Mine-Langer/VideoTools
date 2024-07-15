#include "Remultiplexer.h"

CRemultiplexer::CRemultiplexer()
{
}

CRemultiplexer::~CRemultiplexer()
{
}

bool CRemultiplexer::SetOutput(const char* szOutput, int vWidth, int vHeight, 
    AVChannelLayout chLayout, AVSampleFormat sampleFmt, int sampleRate, int bitRate)
{
    if (0 > avformat_alloc_output_context2(&m_pFormatCtx, nullptr, nullptr, szOutput))
        return false;

    const AVOutputFormat* pOutFmt = m_pFormatCtx->oformat;

    if (pOutFmt->audio_codec) 
    {
        m_audioEncoder.InitAudio(m_pFormatCtx, pOutFmt->audio_codec, sampleFmt, sampleRate, bitRate);
    }

    if (pOutFmt->video_codec) 
    {
        m_videoEncoder.Init(m_pFormatCtx, pOutFmt->video_codec, vWidth, vHeight);
    }

    if (0 > avio_open(&m_pFormatCtx->pb, szOutput, AVIO_FLAG_WRITE))
        return false;

   // av_dump_format(m_pFormatCtx, 0, szOutput, 1);

    return true;
}


void CRemultiplexer::SendFrame(AVFrame* frame, int nType)
{
    if (nType == AVMEDIA_TYPE_VIDEO)
	{
		AVFrame* tframe = nullptr;
		if (frame)
			tframe = av_frame_clone(frame);
        m_videoEncoder.PushFrame(tframe);
    }
    else if (nType == AVMEDIA_TYPE_AUDIO)
    {
        m_audioEncoder.PushFrame(frame); // 此frame已经是alloc 转换过得
    }
}

void CRemultiplexer::Release()
{
    if (m_pFormatCtx) {
        avio_closep(&m_pFormatCtx->pb);
        avformat_free_context(m_pFormatCtx);
        m_pFormatCtx = nullptr;
    }
    m_audioEncoder.Release();
    m_videoEncoder.Release();
}

bool CRemultiplexer::Start(IRemuxEvent* pEvt)
{
    if (!pEvt)
        return false;

    m_pEvent = pEvt;

    m_audioEncoder.Start(this);
    m_videoEncoder.Start(this);

    m_bRun = true;
    m_thread = std::thread(&CRemultiplexer::OnWork, this);

    return true;
}

void CRemultiplexer::SetType(AVType type)
{
    m_avType = type;
}

void CRemultiplexer::WaitFinished()
{
    if (m_thread.joinable())
        m_thread.join();
}

bool CRemultiplexer::VideoEvent(AVPacket* pkt)
{
    m_videoPktQueue.MaxSizePush(pkt, &m_bRun);
    return true;
}

bool CRemultiplexer::AudioEvent(AVPacket* pkt, int64_t pts)
{
    m_audioPktQueue.MaxSizePush(pkt, &m_bRun);
    m_audioPtsQueue.MaxSizePush(pts, &m_bRun);

    return true;
}

void CRemultiplexer::OnWork()
{
    // 写文件头
    avformat_write_header(m_pFormatCtx, nullptr);
    int ret = 0;
    int64_t audioIdx = 0, videoIdx = 0;
    bool endV = false, endA = false;
    while (m_bRun)
    {
        if (m_avType == TAudio)
            ret = WriteAudio(audioIdx);
        else if (m_avType == TVideo)
            ret = WriteImage(videoIdx);
        else if (m_avType == TAll)
            ret = WriteVideo(videoIdx, audioIdx);
        
        if (ret == 0)
            break;
    }
    printf("save file overd.\n");
    av_write_trailer(m_pFormatCtx);

    Release();

    m_pEvent->RemuxEvent(1);
}

int CRemultiplexer::WriteAudio(int64_t& idx)
{
	AVPacket* pkt_a = nullptr; // m_audioEncoder.GetPacketFromFifo(&audioIdx);
	if (m_audioPktQueue.Pop(pkt_a)) 
    {
		if (pkt_a == nullptr)
			return 0;

		m_audioPtsQueue.Pop(idx);

		av_interleaved_write_frame(m_pFormatCtx, pkt_a);
		av_packet_free(&pkt_a);
        idx += 1024;
	}
    return 1;
}

int CRemultiplexer::WriteImage(int64_t& idx)
{
	AVPacket* v_pkt = nullptr;
	if (m_videoPktQueue.Pop(v_pkt)) //v_pkt = m_videoPktQueue.Front();
	{
        if (v_pkt == nullptr)
            return 0;

		av_interleaved_write_frame(m_pFormatCtx, v_pkt);
        idx++;
	}
    return 1;
}

int CRemultiplexer::WriteVideo(int64_t& v_idx, int64_t& a_idx)
{
    int aret = 0, vret = 0;
	// 判断音视频帧时序
	if (0 > av_compare_ts(v_idx, m_videoEncoder.GetTimeBase(), a_idx, m_audioEncoder.GetTimeBase()))
	{
        vret = WriteImage(v_idx);
	}
	else
	{
        aret = WriteAudio(a_idx);
	}
    if (vret == 0 && aret == 0)
        return 0;
    
    return 1;
}