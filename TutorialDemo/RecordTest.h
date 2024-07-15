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

	// ��¼���豸
	bool Open(int type);

	// ��������ļ�
	void SetOutput(const std::string& strFile);

	// ��ʼ¼��
	void Start();

	// ��ͣ¼��
	void Pause();

	// ����¼��
	void Resume();

	// ֹͣ
	void Stop();

protected:
	//  ¼����Ƶ�ӿڻص�����
	virtual void AudioBufEvent(uint8_t* pBuf, int BufSize, int64_t pts) override;

protected:
	void InitAudioRecord();

	void Release();

private:
	// �ط�װ�ӿ�
	CRemultiplexer	m_Remux;

	// ¼���ӿ�
	CAudioRecorder	m_AudioRecoder;

	uint8_t*	m_pSampleBuffer = nullptr;  // ¼������
	uint32_t	m_SampleSize = 0;			
};

