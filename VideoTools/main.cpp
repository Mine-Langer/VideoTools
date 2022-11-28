#if 1
#include "AVMainWnd.h"
#include <QApplication>
#include <QTextCodec>

//https://mnkwxsheem9.haifengsports.club/20221018/rt0EzjWF/index.m3u8
#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    AVMainWnd w;
    w.show();

    return a.exec();
}
#else

#include <QCoreApplication>
#include "AVTools.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    AVTools t;

    t.run();


    return a.exec();
}

#endif