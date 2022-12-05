// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "drawText.h"
#include "FilterVideo.h"

#undef main
int main()
{
	const char* pFileIn = "D:/documents/OneDrive/video/QQ视频搞钱.mp4";

	const char* pFileOut = "haoge.mp4";

	const char* strText = ("牛逼哄哄");

	FilterVideo fv;
	fv.Run(pFileIn, pFileOut, 80, 60, 100, strText);

    return 0;
}
