// audioRecoder.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 

#include "recoder.h"

int main()
{
    std::cout << "Hello World!\n";
    CRecoder recoder;
    recoder.SetConfig(44100, 16, 2);
    recoder.Start();
}
