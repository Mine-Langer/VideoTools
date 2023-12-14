// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "AudioRecorder.h"
#include "AudioTranscode.h"

int audio_idx;
int runAAC(const char* szInput, const char* szOutput);

int main()
{
#if 0
    AudioRecorder recoder("test.aac", "");
	try
	{
		recoder.Open();
		recoder.Start();
		std::this_thread::sleep_for(15 * std::chrono::seconds());

		recoder.Stop();
		std::string reason = recoder.GetLastError();
		if (!reason.empty()) {
			throw std::runtime_error(reason);
		}

	}
	catch (const std::exception&e )
	{
		fprintf(stderr, "[ERROR] %s\n", e.what());
		exit(-1);
	}
#endif

	AudioTranscode audioTrans;
	audioTrans.Run();

    return 0;
}
