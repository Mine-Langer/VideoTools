// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "drawText.h"
#include "FilterVideo.h"
#include "demos.h"
#include "ConvertDemo.h"

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
	ConvertDemo cd;
	cd.Save("output.mp4", "D:/CloudMusic/Brotherhood.mp3");

	system("pause");
    return 0;
}
