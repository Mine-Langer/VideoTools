#pragma once
#include "Remultiplexer.h"
#include "AudioRecorder.h"
#include "AudioDecoder.h"
#include "AudioEncoder.h"
#include "VideoEncoder.h"

class RecordTest :public IRecordEvent
{

public:
	RecordTest();
	~RecordTest();

	// 打开录音设备
	bool Open(int type);

	// 设置输出文件
	void SetOutput(const std::string& strFile);

	// 开始录音
	void Start();

	// 暂停录音
	void Pause();

	// 继续录音
	void Resume();

	// 停止
	void Stop();

protected:
	//  录制音频接口回调函数
	virtual void AudioBufEvent(uint8_t* pBuf, int BufSize, int64_t pts) override;

protected:
	void InitAudioRecord();

	void Release();

private:
	// 重封装接口
	CRemultiplexer	m_Remux;

	// 录音接口
	CAudioRecorder	m_AudioRecoder;

	uint8_t*	m_pSampleBuffer = nullptr;  // 录音缓冲
	uint32_t	m_SampleSize = 0;			
};

