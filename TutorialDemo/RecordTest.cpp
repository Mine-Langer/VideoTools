#include "RecordTest.h"
#include "AudioRecorder.h"
#include "Remultiplexer.h"

RecordTest::RecordTest()
{

}

RecordTest::~RecordTest()
{
	Release();
}

bool RecordTest::Open(int type)
{
	if (!m_AudioRecoder.Init(type))
		return false;

	return true;
}

void RecordTest::SetOutput(const std::string& strFile)
{
	m_Remux.Open(strFile);
	
	m_Remux.SetAudioResampler();
}

void RecordTest::Start()
{
	m_Remux.Start();
	m_AudioRecoder.Start(this);
}

void RecordTest::Pause()
{

}

void RecordTest::Resume()
{

}

void RecordTest::Stop()
{
	m_AudioRecoder;
}

void RecordTest::AudioBufEvent(uint8_t* pBuf, int BufSize, int64_t pts)
{
	m_Remux.SendAudioFrame(pBuf, BufSize);
}

void RecordTest::InitAudioRecord()
{
	
}

void RecordTest::Release()
{

}

