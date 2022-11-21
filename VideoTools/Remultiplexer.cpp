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

    if (0 > avio_open(&m_pFormatCtx->pb, szOutput, AVIO_FLAG_WRITE))
        return false;

    return true;
}


void CRemultiplexer::SendFrame(AVFrame* frame, int nType)
{
    if (nType == AVMEDIA_TYPE_VIDEO)
    {
        AVFrame* tframe = av_frame_clone(frame);
        m_videoFrameQueue.MaxSizePush(tframe, &m_bRun);
    }
    else if (nType == AVMEDIA_TYPE_AUDIO)
    {
        m_audioEncoder.PushFrameToFifo((const uint8_t**)frame->data, frame->nb_samples);
        //m_audioFrameQueue.MaxSizePush(tframe, &m_bRun);
    }
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
    bool endV = false, endA = false;
    while (m_bRun)
    {
        // 判断音视频帧时序
        if (0 > av_compare_ts(videoIdx, m_videoEncoder.GetTimeBase(), audioIdx, m_audioEncoder.GetTimeBase()))
        {
            AVFrame* videoFrame = m_videoFrameQueue.Front();
            if (videoFrame) {
                AVFrame* cvtFrame = m_videoDecoder.ConvertFrame(videoFrame);
                if (!cvtFrame) continue;
                cvtFrame->pts = videoIdx++;

                AVPacket* pkt_v = m_videoEncoder.Encode(cvtFrame);
                if (!pkt_v) continue;

                av_interleaved_write_frame(m_pFormatCtx, pkt_v);
                av_packet_free(&pkt_v);
                av_frame_free(&cvtFrame);
            }
            else
                endV = true;
        }
        else
        {
            AVPacket* pkt_a = m_audioEncoder.GetPacketFromFifo(&audioIdx);
            if (pkt_a) {
                av_interleaved_write_frame(m_pFormatCtx, pkt_a);
                av_packet_free(&pkt_a);
            }
            else
                endA = true;
        }
        if (endV && endA)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    av_write_trailer(m_pFormatCtx);
}
