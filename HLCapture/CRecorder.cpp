#include "CRecorder.h"

CRecorder::CRecorder()
{
	avdevice_register_all();
}

CRecorder::~CRecorder()
{

}

bool CRecorder::Run(const char* szFile)
{
	// 设置视频参数
	if (!InitVideo())
		return false;

	// 设置音频参数
	if (!InitAudio())
		return false;

	// 设置输出参数
	if (!InitOutput(szFile))
		return false;

	// 开始录制
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

bool CRecorder::InitAudio()
{
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

	av_dump_format(OutputFormatCtx, 0, szOutput, 1);

	if (!(OutputFormat->flags & AVFMT_NOFILE))
		if (0 > avio_open(&OutputFormatCtx->pb, szOutput, AVIO_FLAG_WRITE))
			return false;

	if (0 > avformat_write_header(OutputFormatCtx, nullptr))
		return false;

	return true;
}

void CRecorder::Start()
{
	m_bRun = true;
	// 解复用线程
	m_demuxThread = std::thread(&CRecorder::OnDemuxThread, this);

	// 保存数据帧线程
	m_saveThread = std::thread(&CRecorder::OnSaveThread, this);
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
		av_packet_unref(&packet);
	}
}

void CRecorder::OnSaveThread()
{
	AVPacket* vdata = nullptr;
	while (m_bRun)
	{	
		if (!VideoPacketData.Pop(vdata))
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		else
		{
			av_write_frame(OutputFormatCtx, vdata);
			av_packet_free(&vdata);
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

	m_videoEncoder.InitConfig(OutputFormatCtx, VideoWidth, VideoHeight);
	m_videoEncoder.Start(this);
	
	return true;
}

bool CRecorder::InitAudioOutput()
{

	return true;
}

void CRecorder::VideoEvent(AVFrame* vdata)
{
	//AVFrame* vframe = av_frame_clone(vdata);
	m_videoEncoder.Encode(vdata);
}

void CRecorder::VideoEvent(AVPacket* vdata)
{
	AVPacket* vpkt = av_packet_clone(vdata);
	VideoPacketData.Push(vpkt);
}

void CRecorder::AudioEvent(STAudioBuffer* adata)
{

}
