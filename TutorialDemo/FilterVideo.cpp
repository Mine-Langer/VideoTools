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

    if (!OpenFilter(""))
        return false;

    OnDemux();

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
    char szArgs[512] = { 0 };
    const AVFilter* filterSrc = avfilter_get_by_name("buffer");
    const AVFilter* filterSink = avfilter_get_by_name("buffersink");
    const AVFilter* filterText = avfilter_get_by_name("drawtext");

    //AVFilterInOut* filterOutput = avfilter_inout_alloc();
    //AVFilterInOut* filterInput = avfilter_inout_alloc();

    m_pFilterGraph = avfilter_graph_alloc();
    
    AVRational tb = m_codec_ctx_out->time_base;
    AVRational ts = m_codec_ctx_out->sample_aspect_ratio;
    sprintf_s(szArgs, 512, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", 
        m_codec_ctx_out->width, m_codec_ctx_out->height, m_codec_ctx_out->pix_fmt, tb.num, tb.den, ts.num, ts.den);

    if (0 > avfilter_graph_create_filter(&m_pFilterCtxSrc, filterSrc, "in", szArgs, nullptr, m_pFilterGraph))
        return false;

    if (0 > avfilter_graph_create_filter(&m_pFilterCtxText, filterText, "drawtext", szFilterDesc, nullptr, m_pFilterGraph))
        return false;

    if (0 > avfilter_graph_create_filter(&m_pFilterCtxSink, filterSink, "out", nullptr, nullptr, m_pFilterGraph))
        return false;

    av_opt_set_bin(m_pFilterCtxSink, "pix_fmts", (uint8_t*)&m_codec_ctx_out->pix_fmt, sizeof(m_codec_ctx_out->pix_fmt), AV_OPT_SEARCH_CHILDREN);

    if (0 > avfilter_link(m_pFilterCtxSrc, 0, m_pFilterCtxText, 0))
        return false;

    if (0 > avfilter_link(m_pFilterCtxText, 0, m_pFilterCtxSink, 0))
        return false;

    if (avfilter_graph_config(m_pFilterGraph, nullptr))
        return false;

    char* szDump = avfilter_graph_dump(m_pFilterGraph, nullptr);

    return true;
}

void FilterVideo::OnDemux()
{
    AVPacket pkt;
    AVFrame* srcFrame = av_frame_alloc();
    AVFrame* filterFrame = av_frame_alloc();
    AVPacket* dstPacket = av_packet_alloc();

    while (true)
    {
        if (0 > av_read_frame(m_fmt_ctx_in, &pkt))
            break;

        if (pkt.stream_index != m_videoIndex)
            continue;

        if (0 > avcodec_send_packet(m_codec_ctx_in, &pkt))
            continue;

        if (0 > avcodec_receive_frame(m_codec_ctx_in, srcFrame))
            continue;

        av_buffersrc_add_frame(m_pFilterCtxSrc, srcFrame);

        av_buffersink_get_frame(m_pFilterCtxSink, filterFrame);


        if (0 > avcodec_send_frame(m_codec_ctx_out, filterFrame))
            continue;

        if (0 > avcodec_receive_packet(m_codec_ctx_out, dstPacket))
            continue;
    }
}
