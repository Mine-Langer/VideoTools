#include "AudioRecorder.h"

const AVSampleFormat requireAudioFmt = AV_SAMPLE_FMT_FLTP;
AudioRecorder::AudioRecorder(std::string filePath, std::string device)
	:outFile(filePath),deviceName(device)
{
	avdevice_register_all();
}

AudioRecorder::~AudioRecorder()
{
	Stop();
}

void AudioRecorder::Open()
{
	///input context ///////////////////////////////////////////////////////////////////////
	AVDictionary* options = nullptr;
	int ret = 0;

	deviceName = listAVDevice.DS_GetDefaultDevice("a");
	if (deviceName == "") {
		throw std::runtime_error("Fail to get default audio device, maybe no microphone.");
	}
	deviceName = "audio=" + deviceName;
	const AVInputFormat* inputFormat = av_find_input_format("dshow");

	ret = avformat_open_input(&audioInFormatCtx, deviceName.c_str(), inputFormat, &options);
	if (ret!= 0)
		throw std::runtime_error("Couldn't open input audio stream.");

	if (0 > avformat_find_stream_info(audioInFormatCtx, nullptr))
		throw std::runtime_error("Couldn't find audio stream information.");

	const AVCodec* audioInCodec = nullptr;
	int audioIdx = av_find_best_stream(audioInFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &audioInCodec, 0);
	if (0 > audioIdx)
		throw std::runtime_error("Couldn't find a audio stream.");
	
	audioInStream = audioInFormatCtx->streams[audioIdx];
	audioInCodecCtx = avcodec_alloc_context3(audioInCodec);
	avcodec_parameters_to_context(audioInCodecCtx, audioInStream->codecpar);

	if (0 > avcodec_open2(audioInCodecCtx, audioInCodec, nullptr))
		throw std::runtime_error("Could not open video codec.");

	swr_alloc_set_opts2(&audioConverter,
		&audioInCodecCtx->ch_layout,
		requireAudioFmt,
		audioInCodecCtx->sample_rate,
		&audioInCodecCtx->ch_layout,
		(AVSampleFormat)audioInStream->codecpar->format,
		audioInStream->codecpar->sample_rate,
		0, nullptr);
	swr_init(audioConverter);

	audioFifo = av_audio_fifo_alloc(requireAudioFmt, audioInCodecCtx->ch_layout.nb_channels,
		audioInCodecCtx->sample_rate * 2);
	
	/// output context ///////////////////////////////////////////
	ret = avformat_alloc_output_context2(&audioOutFormatCtx, nullptr, "adts", nullptr);
	if (0 > ret)
		throw std::runtime_error("Failed to alloc ouput context");

	ret = avio_open2(&audioOutFormatCtx->pb, outFile.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
	if (0 > ret)
		throw std::runtime_error("Fail to open output file.");

	const AVCodec* audioOutCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!audioOutCodec)
		throw std::runtime_error("Fail to find aac encoder. Please check your DLL.");

	audioOutCodecCtx = avcodec_alloc_context3(audioOutCodec);
	audioOutCodecCtx->ch_layout = audioInStream->codecpar->ch_layout;
	audioOutCodecCtx->sample_rate = audioInStream->codecpar->sample_rate;
	audioOutCodecCtx->sample_fmt = audioOutCodec->sample_fmts[0];
	audioOutCodecCtx->bit_rate = 32000;
	audioOutCodecCtx->time_base.num = 1;
	audioOutCodecCtx->time_base.den = audioOutCodecCtx->sample_rate;

	if (audioOutFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		audioOutCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (0 > avcodec_open2(audioOutCodecCtx, audioOutCodec, nullptr))
		throw std::runtime_error("Fail to open ouput audio encoder.");

	//Add a new stream to output,should be called by the user before avformat_write_header() for muxing
	audioOutStream = avformat_new_stream(audioOutFormatCtx, audioOutCodec);

	if (audioOutStream == nullptr)
		throw std::runtime_error("Fail to new a audio stream.");

	avcodec_parameters_from_context(audioOutStream->codecpar, audioOutCodecCtx);

	// write file header
	if (avformat_write_header(audioOutFormatCtx, nullptr) < 0)
		throw std::runtime_error("Fail to write header for audio.");
}

void AudioRecorder::Start()
{
	audioThread = std::thread([this]() {
		this->isRun = true;
		puts("Start record."); fflush(stdout);
		try {
			this->StartEncode();
		}
		catch (std::exception e) {
			this->failReason = e.what();
		}
	});
}

void AudioRecorder::Stop()
{
	bool r = isRun.exchange(false);
	if (!r) return; //avoid run twice
	if (audioThread.joinable())
		audioThread.join();

	int ret = av_write_trailer(audioOutFormatCtx);
	if (ret < 0) 
		throw std::runtime_error("can not write file trailer.");
	avio_close(audioOutFormatCtx->pb);

	swr_free(&audioConverter);
	av_audio_fifo_free(audioFifo);

	avcodec_free_context(&audioInCodecCtx);
	avcodec_free_context(&audioOutCodecCtx);

	avformat_close_input(&audioInFormatCtx);
	avformat_free_context(audioOutFormatCtx);
	puts("Stop record."); fflush(stdout);
}

std::string AudioRecorder::GetLastError()
{
	return failReason;
}

void AudioRecorder::StartEncode()
{
	AVFrame* inputFrame = av_frame_alloc();
	AVPacket* inputPacket = av_packet_alloc();

	AVPacket* outputPacket = av_packet_alloc();
	uint64_t  frameCount = 0;

	int ret;

	while (isRun) {

		//  decoding
		ret = av_read_frame(audioInFormatCtx, inputPacket);
		if (ret < 0) {
			throw std::runtime_error("can not read frame");
		}
		ret = avcodec_send_packet(audioInCodecCtx, inputPacket);
		if (ret < 0) {
			throw std::runtime_error("can not send pkt in decoding");
		}
		ret = avcodec_receive_frame(audioInCodecCtx, inputFrame);
		if (ret < 0) {
			throw std::runtime_error("can not receive frame in decoding");
		}
		//--------------------------------
		// encoding

		uint8_t** cSamples = nullptr;
		ret = av_samples_alloc_array_and_samples(&cSamples, NULL, audioOutCodecCtx->ch_layout.nb_channels, inputFrame->nb_samples, requireAudioFmt, 0);
		if (ret < 0) {
			throw std::runtime_error("Fail to alloc samples by av_samples_alloc_array_and_samples.");
		}
		ret = swr_convert(audioConverter, cSamples, inputFrame->nb_samples, (const uint8_t**)inputFrame->extended_data, inputFrame->nb_samples);
		if (ret < 0) {
			throw std::runtime_error("Fail to swr_convert.");
		}
		if (av_audio_fifo_space(audioFifo) < inputFrame->nb_samples) throw std::runtime_error("audio buffer is too small.");

		ret = av_audio_fifo_write(audioFifo, (void**)cSamples, inputFrame->nb_samples);
		if (ret < 0) {
			throw std::runtime_error("Fail to write fifo");
		}

		av_freep(&cSamples[0]);

		av_frame_unref(inputFrame);
		av_packet_unref(inputPacket);

		while (av_audio_fifo_size(audioFifo) >= audioOutCodecCtx->frame_size) {
			AVFrame* outputFrame = av_frame_alloc();
			outputFrame->nb_samples = audioOutCodecCtx->frame_size;
			outputFrame->ch_layout = audioInCodecCtx->ch_layout;
			outputFrame->format = requireAudioFmt;
			outputFrame->sample_rate = audioOutCodecCtx->sample_rate;

			ret = av_frame_get_buffer(outputFrame, 0);
			assert(ret >= 0);
			ret = av_audio_fifo_read(audioFifo, (void**)outputFrame->data, audioOutCodecCtx->frame_size);
			assert(ret >= 0);

			outputFrame->pts = frameCount * audioOutStream->time_base.den * 1024 / audioOutCodecCtx->sample_rate;

			ret = avcodec_send_frame(audioOutCodecCtx, outputFrame);
			if (ret < 0) {
				throw std::runtime_error("Fail to send frame in encoding");
			}
			av_frame_free(&outputFrame);
			ret = avcodec_receive_packet(audioOutCodecCtx, outputPacket);
			if (ret == AVERROR(EAGAIN)) {
				continue;
			}
			else if (ret < 0) {
				throw std::runtime_error("Fail to receive packet in encoding");
			}


			outputPacket->stream_index = audioOutStream->index;
			outputPacket->duration = audioOutStream->time_base.den * 1024 / audioOutCodecCtx->sample_rate;
			outputPacket->dts = outputPacket->pts = frameCount * audioOutStream->time_base.den * 1024 / audioOutCodecCtx->sample_rate;

			frameCount++;

			ret = av_write_frame(audioOutFormatCtx, outputPacket);
			av_packet_unref(outputPacket);

		}



	}

	av_packet_free(&inputPacket);
	av_packet_free(&outputPacket);
	av_frame_free(&inputFrame);
	printf("encode %lu audio packets in total.\n", frameCount);
}
