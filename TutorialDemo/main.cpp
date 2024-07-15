// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "RecordTest.h"
#include "Transcode.h"
#include "AudioRecorder.h"

int _tmain(int argc, const TCHAR* argv[])
{
#if 0
	CAudioRecorder ar(SOUNDCARD);
	ar.Start(nullptr);
#endif

#if 1
	{
		RecordTest record;
		record.Open(SOUNDCARD);

		record.SetOutput("record_audio.mp3");

		record.Start();

		_tsystem(_T("pause"));
	}
#endif

#if 0
	{
		CTranscode trans;
		trans.OpenInput("F:/CloudMusic/John Dreamer - Brotherhood.mp3");
		trans.SetOutput("test.mp3");
		trans.Run();
		_tsystem(_T("pause"));
	}
#endif
    _tsystem(_T("pause"));
    return 0;
}
