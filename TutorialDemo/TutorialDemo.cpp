// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "demos.h"
#include <Windows.h>

int main(int argc, char* argv[])
{
//    CScaleImage scale;
//    scale.Scale("scale_test.yuv", "320x240");

//    ExtractMvs e("D:/documents/OneDrive/video/tbbjszwb.mp4");

//    CDemos demo;
//    demo.AvioReading("D:/documents/OneDrive/video/tbbjszwb.mp4");
    int iSec = GetDoubleClickTime();


    //TAAC aac;
    CTransAAC aac;
    aac.Run("D:/CloudMusic/The Titan.mp3", "output.aac");
    
    system("pause");
    return 0;
}
