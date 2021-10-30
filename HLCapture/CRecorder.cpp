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
	/*{ 
		AVFormatContext* fmtctx = avformat_alloc_context();
		AVDictionary* options = nullptr;
		av_dict_set(&options, "list_devices", "true", 0);
		AVInputFormat* ifmt = av_find_input_format("dshow");
		int ret = avformat_open_input(&fmtctx, "video=dummy", ifmt, &options);
		if (avformat_open_input(&fmtctx, "1", ifmt, NULL) != 0)
			return false;
	}*/
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
static char* dup_wchar_to_utf8(const wchar_t* w)
{
	char* s = NULL;
	int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	s = (char*)av_malloc(l);
	if (s)
		WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
	return s;
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
	AVInputFormat* ifmt = av_find_input_format("dshow");
	if (ifmt == nullptr)
		return false;

	const WCHAR* pszDevName[] = { L"audio=立体声混音 (Realtek(R) Audio)", L"audio=麦克风 (Realtek High Definition Audio)", L"audio=Stereo Mix (Realtek High Defini", L"audio=virtual-audio-capturer"};
	AudioFormatCtx = avformat_alloc_context();
	char* szDevName = dup_wchar_to_utf8(pszDevName[0]);
	if (0 != avformat_open_input(&AudioFormatCtx, szDevName, ifmt, nullptr))
		return false;

	if (0 > avformat_find_stream_info(AudioFormatCtx, nullptr))
		return false;

	AVCodec* codec = nullptr;
	AudioIndex = av_find_best_stream(AudioFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	if (AudioIndex == -1)
		return false;

	if (!m_audioDecoder.Open(AudioFormatCtx->streams[AudioIndex]))
		return false;
	m_audioDecoder.SetConfig();
	m_audioDecoder.Start(this);

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

void CRecorder::Stop()
{

}

void CRecorder::Start()
{
	m_bRun = true;
	// 录像解复用线程
	m_demuxVThread = std::thread(&CRecorder::OnDemuxVideoThread, this);
	// 录音解复用线程
	m_demuxAThread = std::thread(&CRecorder::OnDemuxAudioThread, this);
	// 保存数据帧线程
	m_saveThread = std::thread(&CRecorder::OnSaveThread, this);
}

void CRecorder::OnDemuxVideoThread()
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

		av_packet_unref(&packet);
	}
}

void CRecorder::OnDemuxAudioThread()
{
	AVPacket packet = { 0 };
	while (m_bRun)
	{
		if (0 > av_read_frame(AudioFormatCtx, &packet))
			break;
		
		if (packet.stream_index == AudioIndex)
			m_audioDecoder.SendPacket(&packet);

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
	av_write_trailer(OutputFormatCtx);
}

bool CRecorder::InitVideoOutput()
{
	int VideoWidth, VideoHeight; 
	AVPixelFormat VideoFormat;
	m_videoDecoder.GetParameter(VideoWidth, VideoHeight, VideoFormat);

	m_videoEncoder.InitConfig(OutputFormatCtx, VideoWidth, VideoHeight);
	m_videoEncoder.Start(this);
	
	return true;
}

bool CRecorder::InitAudioOutput()
{
	m_audioEncoder.InitAudio(OutputFormatCtx, OutputFormat->audio_codec, &m_audioDecoder);
	m_audioEncoder.Start(this);

	return true;
}

void CRecorder::VideoEvent(AVFrame* vdata)
{
	m_videoEncoder.Encode(vdata);
}

void CRecorder::VideoEvent(AVPacket* vdata)
{
	AVPacket* vpkt = av_packet_clone(vdata);
	VideoPacketData.Push(vpkt);
}

void CRecorder::AudioEvent(STAudioBuffer* adata)
{
	AVFrame* frame = av_frame_alloc();
	m_audioEncoder.PushFrame(frame);
}

void CRecorder::AudioEvent(AVPacket* adata)
{

}
