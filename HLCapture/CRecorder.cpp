#include "CRecorder.h"

CRecorder::CRecorder()
{
	avdevice_register_all();
}

CRecorder::~CRecorder()
{

}

bool CRecorder::Run()
{
	if (!InitVideo())
		return false;

	Start();

	return true;
}

bool CRecorder::InitVideo()
{
	AVDictionary* options = nullptr;
	StreamFrameRate = 25;
	av_dict_set(&options, "framerate", "25", 0);
	av_dict_set(&options, "offset_x", "0", 0);
	av_dict_set(&options, "offset_y", "0", 0);
	AVInputFormat* ifmt = av_find_input_format("gdigrab");
	if (0 != avformat_open_input(&VideoFormatCtx, "desktop", ifmt, &options))
		return false;

	if (0 > avformat_find_stream_info(VideoFormatCtx, nullptr))
		return false;

	AVCodec* videoCodec = nullptr;
	VideoIndex = av_find_best_stream(VideoFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &videoCodec, 0);
	if (VideoIndex == -1)
		return false;

	if (!m_videoDecoder.Open(VideoFormatCtx->streams[VideoIndex]))
		return false;

	if (!m_videoDecoder.SetConfig(-1, -1, AV_PIX_FMT_YUV420P))
		return false;

	m_videoDecoder.Start(this);

	return true;
}

bool CRecorder::InitOutput(const char* szOutput)
{
	if (0 > avformat_alloc_output_context2(&OutputFormatCtx, nullptr, nullptr, szOutput))
		return false;

	OutputFormat = OutputFormatCtx->oformat;

	if (OutputFormat->video_codec != AV_CODEC_ID_NONE)
		InitVideoOutput();

	if (OutputFormat->audio_codec != AV_CODEC_ID_NONE)
		InitAudioOutput();

	return true;
}

void CRecorder::Start()
{
	m_bRun = true;
	// 解复用线程
	m_demuxThread = std::thread(&CRecorder::OnDemuxThread, this);
}

void CRecorder::OnDemuxThread()
{
	AVPacket packet;
	while (m_bRun)
	{
		if (0 > av_read_frame(VideoFormatCtx, &packet))
			break;
		
		if (packet.stream_index == VideoIndex)
		{
			m_videoDecoder.SendPacket(&packet);
		}

		if (packet.stream_index == AudioIndex)
		{

		}
	}
}

bool CRecorder::InitVideoOutput()
{
	AVCodec* codec = avcodec_find_encoder(OutputFormat->video_codec);
	if (codec == nullptr)
		return false;

	AVStream* pStream = avformat_new_stream(OutputFormatCtx, nullptr);
	pStream->id = OutputFormatCtx->nb_streams - 1;
	pStream->time_base = { 1, StreamFrameRate };

	int VideoWidth, VideoHeight; 
	AVPixelFormat VideoFormat;
	m_videoDecoder.GetParameter(VideoWidth, VideoHeight, VideoFormat);
	AVCodecContext* outputVideoCodecCtx = avcodec_alloc_context3(codec);
	outputVideoCodecCtx->codec_id = OutputFormat->video_codec;
	outputVideoCodecCtx->bit_rate = 400000;
	outputVideoCodecCtx->width = VideoWidth;
	outputVideoCodecCtx->height = VideoHeight;
	outputVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	outputVideoCodecCtx->time_base = pStream->time_base;
	outputVideoCodecCtx->gop_size = 12;

	if (OutputFormat->flags & AVFMT_GLOBALHEADER)
		outputVideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	AVDictionary* param = nullptr;
	if (outputVideoCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		av_dict_set(&param, "preset", "slow", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
	}

	if (0 > avcodec_open2(outputVideoCodecCtx, codec, &param))
		return false;

	avcodec_parameters_from_context(pStream->codecpar, outputVideoCodecCtx);

	return true;
}

bool CRecorder::InitAudioOutput()
{

	return true;
}

void CRecorder::VideoEvent(AVFrame* vdata)
{
	AVFrame* vframe = av_frame_clone(vdata);
	VideoFrameData.Push(vframe);
}

void CRecorder::AudioEvent(STAudioBuffer* adata)
{

}
