#include "DemuxAudio.h"

#define WFILE 0

DemuxAudio::~DemuxAudio()
{
    Release();
}

bool DemuxAudio::Open(const char* szinput)
{
    if (0 != avformat_open_input(&InputFormatCtx, szinput, nullptr, nullptr))
        return false;

    if (0 > avformat_find_stream_info(InputFormatCtx, nullptr))
        return false;

    AudioIndex = av_find_best_stream(InputFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (AudioIndex < 0)
        return false;

    AVStream* pStream = InputFormatCtx->streams[AudioIndex];
    InputCodecCtx = avcodec_alloc_context3(nullptr);
    const AVCodec* pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
    if (!pCodec)
        return false;

    if (0 > avcodec_parameters_to_context(InputCodecCtx, pStream->codecpar))
        return false;

    if (0 > avcodec_open2(InputCodecCtx, pCodec, nullptr))
        return false;

    InputFrame = av_frame_alloc();
    InputPkt = av_packet_alloc();

    AVChannelLayout OutChannelLayout;
    av_channel_layout_default(&OutChannelLayout, 2);
    OutChannelLayout.nb_channels = 2; // AV_CH_LAYOUT_STEREO;
    AVSampleFormat OutputSample = AV_SAMPLE_FMT_S16;
    int OutputRate = 44100;
    if (0 > swr_alloc_set_opts2(&SwrCtx, &OutChannelLayout, OutputSample, OutputRate,
        &InputCodecCtx->ch_layout, InputCodecCtx->sample_fmt, InputCodecCtx->sample_rate, 0, nullptr))
        return false;
    if (!SwrCtx)
        return false;

    if (0 > swr_init(SwrCtx))
        return false;

    return true;
}

bool DemuxAudio::SetOutput(const char* szOutput)
{
    if (0 > avformat_alloc_output_context2(&OutputFormatCtx, nullptr, nullptr, szOutput))
        return false;

    const AVOutputFormat* OutputFormat = OutputFormatCtx->oformat;
    if (!OutputFormat->audio_codec)
        return false;

    const AVCodec* pCodec = avcodec_find_encoder(OutputFormat->audio_codec);
    if (!pCodec)
        return false;

    OutputCodecCtx = avcodec_alloc_context3(pCodec);
    OutputCodecCtx->sample_rate = InputCodecCtx->sample_rate;
    OutputCodecCtx->sample_fmt = pCodec->sample_fmts[0];
    av_channel_layout_default(&OutputCodecCtx->ch_layout, 2);
    OutputCodecCtx->time_base.den = OutputCodecCtx->sample_rate;
    OutputCodecCtx->time_base.num = 1;


    return true;
}


void DemuxAudio::Run()
{
    int err = 0;
    int idx = 0;
    AVStream* pStream = InputFormatCtx->streams[AudioIndex];

    AudioPlayer.Start();

#if WFILE
    DWORD dwRet = 0;
    HANDLE hFile = CreateFile(_T("Titan.pcm"), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif

    uint8_t* outBuf = (uint8_t*)av_malloc(2 * 2 * 44100);
    while (true)
    {
        err = av_read_frame(InputFormatCtx, InputPkt);
        if (err < 0)
            break;

        if (InputPkt->stream_index != AudioIndex)
            continue;

        if (0 != avcodec_send_packet(InputCodecCtx, InputPkt))
            continue;

        if (0 != avcodec_receive_frame(InputCodecCtx, InputFrame))
            continue;

        printf("packet timestamp:%.2f duration:%.2f\t", av_q2d(pStream->time_base) * InputPkt->pts, av_q2d(pStream->time_base)*InputPkt->duration);


        // convert
        int swrSize = swr_convert(SwrCtx, &outBuf, InputFrame->nb_samples, (const uint8_t**)InputFrame->extended_data, InputFrame->nb_samples);
        if (0 > swrSize)
            continue;

        int outChannels = 2;// AV_CH_LAYOUT_STEREO;// av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
        int data_size = swrSize * outChannels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

#if WFILE
        WriteFile(hFile, outBuf, data_size, &dwRet, nullptr);
#endif

        printf("frame index:%d,  frame size:%d pts:%lld datasize:%d \n", idx++, InputFrame->nb_samples, InputFrame->pts, data_size);
        AudioPlayer.PushPCMBuf(outBuf, data_size);

        av_frame_unref(InputFrame);
        av_packet_unref(InputPkt);
    }

#if WFILE
    CloseHandle(hFile);
#endif

    av_free(outBuf);
}

void DemuxAudio::Release()
{
    if (InputFormatCtx) {
        avformat_free_context(InputFormatCtx);
        InputFormatCtx = nullptr;
    }

    if (InputCodecCtx) {
        avcodec_free_context(&InputCodecCtx);
        InputCodecCtx = nullptr;
    }

    if (InputFrame) {
        av_frame_free(&InputFrame);
        InputFrame = nullptr;
    }

    if (InputPkt) {
        av_packet_free(&InputPkt);
        InputPkt = nullptr;
    }
}
