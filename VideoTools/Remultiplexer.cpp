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

    if (pOutFmt->audio_codec) {

        m_audioEncoder.InitAudio(m_pFormatCtx, pOutFmt->audio_codec, chLayout, sampleFmt, sampleRate);
    }

    if (pOutFmt->video_codec) {

        m_videoEncoder.Init(m_pFormatCtx, pOutFmt->video_codec, vWidth, vHeight);
    }

    return true;
}


void CRemultiplexer::SendFrame(AVFrame* frame, int nType)
{

}

void CRemultiplexer::Release()
{

}

bool CRemultiplexer::Start()
{
    m_bRun = true;
    m_thread = std::thread(&CRemultiplexer::OnWork, this);

    return false;
}

void CRemultiplexer::OnWork()
{
    // 写文件头
    avformat_write_header(m_pFormatCtx, nullptr);

    int audioIdx = 0, videoIdx = 0;

    while (m_bRun)
    {
        // 判断音视频帧时序
        if (0 > av_compare_ts(videoIdx, m_videoEncoder.GetTimeBase(), audioIdx, m_audioEncoder.GetTimeBase()))
        {

        }
        else
        {

        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    av_write_trailer(m_pFormatCtx);
}
