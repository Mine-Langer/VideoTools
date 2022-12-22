#include "Remultiplexer.h"

CRemultiplexer::CRemultiplexer()
{
}

CRemultiplexer::~CRemultiplexer()
{
}

bool CRemultiplexer::SetOutput(const char* szOutput, int vWidth, int vHeight, AVChannelLayout chLayout, AVSampleFormat sampleFmt, int sampleRate)
{
    if (0 > avformat_alloc_output_context2(&m_pFormatCtx, nullptr, nullptr, szOutput))
        return false;

    const AVOutputFormat* pOutFmt = m_pFormatCtx->oformat;

    if (pOutFmt->audio_codec) 
    {
        m_audioEncoder.InitAudio(m_pFormatCtx, pOutFmt->audio_codec);
    }

    if (pOutFmt->video_codec) 
    {
        m_videoEncoder.Init(m_pFormatCtx, pOutFmt->video_codec, vWidth, vHeight);
    }

    if (0 > avio_open(&m_pFormatCtx->pb, szOutput, AVIO_FLAG_WRITE))
        return false;

    av_dump_format(m_pFormatCtx, 0, szOutput, 1);

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
        if (frame)
            m_audioEncoder.PushFrame(frame);
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

    return false;
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

    int64_t audioIdx = 0, videoIdx = 0;
    bool endV = false, endA = false;
    while (m_bRun)
    {
        if (m_videoPktQueue.Empty()){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        // 判断音视频帧时序
        if (0 > av_compare_ts(videoIdx, m_videoEncoder.GetTimeBase(), audioIdx, m_audioEncoder.GetTimeBase()))
        {
            AVPacket* v_pkt = nullptr;
            if (m_videoPktQueue.Pop(v_pkt)) //v_pkt = m_videoPktQueue.Front();
            {
                av_interleaved_write_frame(m_pFormatCtx, v_pkt);
                videoIdx++;
            }
        }
        else
        {
            AVPacket* pkt_a = nullptr; // m_audioEncoder.GetPacketFromFifo(&audioIdx);
            if (m_audioPktQueue.Pop(pkt_a)) {
                if (pkt_a == nullptr)
                    break;

                m_audioPtsQueue.Pop(audioIdx);

                av_interleaved_write_frame(m_pFormatCtx, pkt_a);
                av_packet_free(&pkt_a);
                audioIdx += 1024;
            }
            else
                endA = true;
        }
        if (endV && endA)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    printf("save file overd.\n");
    av_write_trailer(m_pFormatCtx);

    Release();

    m_pEvent->RemuxEvent(1);
}
