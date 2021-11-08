// audioRecoder.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// https://blog.csdn.net/ZeroLing96/article/details/102801801

#include "recoder.h"

int main()
{
    std::cout << "Hello World!\n";
    CRecoder recoder;
    if (recoder.Init())
    {
        recoder.Start();
    }

    while (1)
    {
        Sleep(50);
    }
}
