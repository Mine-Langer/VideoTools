#include "FilterVideo.h"

FilterVideo::FilterVideo()
{
}

FilterVideo::~FilterVideo()
{
}

bool FilterVideo::Run(const char* szInput, const char* szOutput, int x, int y, int size, const char* szText)
{
    if (!OpenInput(szInput))
        return false;

    if (!OpenOutput(szOutput))
        return false;


    return true;
}

bool FilterVideo::OpenInput(const char* szInput)
{
    if (0 != avformat_open_input(&m_fmt_ctx_in, szInput, nullptr, nullptr))
        return false;

    if (0 > avformat_find_stream_info(m_fmt_ctx_in, nullptr))
        return false;

    const AVCodec* pCodec = nullptr;
    m_videoIndex = av_find_best_stream(m_fmt_ctx_in, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (m_videoIndex < 0)
        return false;
    
    m_codec_ctx_in = avcodec_alloc_context3(pCodec);
    if (0 > avcodec_parameters_to_context(m_codec_ctx_in, m_fmt_ctx_in->streams[m_videoIndex]->codecpar))
        return false;

    if (0 > avcodec_open2(m_codec_ctx_in, pCodec, nullptr))
        return false;

    return true;
}

bool FilterVideo::OpenOutput(const char* szOutput)
{
    if (0 > avformat_alloc_output_context2(&m_fmt_ctx_out, nullptr, nullptr, szOutput)) 
        return false;

    const AVOutputFormat* out_fmt = m_fmt_ctx_out->oformat;
    if (!out_fmt->video_codec)
        return false;

    if (0 > avio_open(&m_fmt_ctx_out->pb, szOutput, AVIO_FLAG_WRITE))
        return false;

    const AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    m_codec_ctx_out = avcodec_alloc_context3(pCodec);
    m_codec_ctx_out->codec_id = AV_CODEC_ID_H264;
    m_codec_ctx_out->width = m_codec_ctx_in->width;
    m_codec_ctx_out->height = m_codec_ctx_in->height;
    m_codec_ctx_out->pix_fmt = m_codec_ctx_in->pix_fmt;
    m_codec_ctx_out->bit_rate = 4000000;
    m_codec_ctx_out->time_base = { 1, 25 };
    m_codec_ctx_out->gop_size = 12;

    av_opt_set(m_codec_ctx_out->priv_data, "b-pyramid", "none", 0);
    av_opt_set(m_codec_ctx_out->priv_data, "preset", "superfast", 0);
    av_opt_set(m_codec_ctx_out->priv_data, "tune", "zerolatency", 0);

    if (out_fmt->flags & AVFMT_GLOBALHEADER)
        m_codec_ctx_out->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    AVStream* pStream = avformat_new_stream(m_fmt_ctx_out, pCodec);
    pStream->time_base = m_codec_ctx_out->time_base;
    pStream->id = m_fmt_ctx_out->nb_streams - 1;

    if (0 > avcodec_parameters_from_context(pStream->codecpar, m_codec_ctx_out))
        return false;

    if (0 > avcodec_open2(m_codec_ctx_out, pCodec, nullptr))
        return false;

    return true;
}

bool FilterVideo::OpenFilter(const char* szFilterDesc)
{
    const AVFilter* filterSrc = avfilter_get_by_name("buffer");
    const AVFilter* filterSink = avfilter_get_by_name("buffersink");
    const AVFilter* filterText = avfilter_get_by_name("drawtext");

    return false;
}
