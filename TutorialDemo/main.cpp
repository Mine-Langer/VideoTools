// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "demos.h"
#include "DemuxAudio.h"


int main(int argc, char* argv[])
{
//    CScaleImage scale;
//    scale.Scale("scale_test.yuv", "320x240");

//    ExtractMvs e("D:/documents/OneDrive/video/tbbjszwb.mp4");

//    CDemos demo;
//    demo.AvioReading("D:/documents/OneDrive/video/tbbjszwb.mp4");
    int iSec = GetDoubleClickTime();

    int a = 64 / 8;
    int b = 64 >> 3;

    //CAudioTranslate aac;
    //CTransAAC aac;
    //aac.Run("D:/CloudMusic/The Titan.mp3", "output.aac");

    //OnFilterAudio("64.89");

    //OnFilteringAudio("D:/CloudMusic/The Titan.mp3");

    //PcmPlay player;
    //player.Test();

    /*CAudioConvert audioCvt;
    if (!audioCvt.Open("D:/CloudMusic/The Titan.mp3"))
        return -1;
    
    audioCvt.SetOption(2, 96000);

    audioCvt.Save("Titan.aac");

    audioCvt.Start();

    audioCvt.Close();*/


    //DSPlayer player;
    //player.Start();

    //Sleep(-1);

    /*DemuxAudio demux;
    if (demux.Open("d:/CloudMusic/The Titan.mp3"))
    {
        demux.SetOutput("Titan.aac");
        demux.Run();
    }
    demux.Release();
    */

    //CM3u8Test m;
    //m.OpenUrl("https://mnkwxsheem9.haifengsports.club/20221018/rt0EzjWF/index.m3u8");

    system("pause");
    return 0;
}
