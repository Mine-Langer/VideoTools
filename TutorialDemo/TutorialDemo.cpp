// TutorialDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "demos.h"

int main(int argc, char* argv[])
{
    CScaleImage scale;
    scale.Scale("scale_test.mp4", "320x240");

    system("pause");
    return 0;
}
