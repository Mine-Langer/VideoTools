// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "drawText.h"
#include "FilterVideo.h"
#include "demos.h"
#include "ConvertDemo.h"
#include "ImageConvert.h"

#undef main
int main()
{
	/*CDrawText drawtext;
	const char* pFileIn = "D:/documents/OneDrive/video/QQ视频搞钱.mp4";

	const char* pFileOut = "haoge.mp4";

	const char* strText = ("牛逼哄哄");

	drawtext.StartDrawText(pFileIn, pFileOut, 100,100,48,strText);
	drawtext.WaitFinish();*/

	//CTransAAC t;
	//t.Run("D:/CloudMusic/Brotherhood.mp3", "output.aac");

#if 0
	ConvertDemo cd;
	cd.Save("output.mp4", "G:/CloudMusic/The Titan.mp3"); // G:\CloudMusic\The Titan.mp3  D:/CloudMusic/Brotherhood.mp3
#elif 0
	HWDecode hwd;
	hwd.Open("D:/documents/OneDrive/video/QQ视频搞钱.mp4");
#elif 1
	ImageConvert imgCvt;
	imgCvt.Open("D:/tools/RoboticProcessAutomation/src/resource/Click_screen.gif");
	imgCvt.SetOutput();
	imgCvt.Work();

#endif

	system("pause");
    return 0;
}
