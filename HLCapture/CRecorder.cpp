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

void CRecorder::Start()
{
	m_bRun = true;
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

void CRecorder::VideoEvent(AVFrame* vdata)
{
	AVFrame* vframe = av_frame_clone(vdata);
	VideoFrameData.Push(vframe);
}

void CRecorder::AudioEvent(STAudioBuffer* adata)
{

}
