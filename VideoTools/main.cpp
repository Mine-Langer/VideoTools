#include "Common.h"
#include "AVMainWnd.h"
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QTextStream>

//https://mnkwxsheem9.haifengsports.club/20221018/rt0EzjWF/index.m3u8
#undef main
int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    
    QFile qssFile(":/AvTools/res/style.qss");
    if (qssFile.exists())
    {
        qssFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&qssFile);
        in.setCodec("UTF-8");
        QString qss = in.readAll();
        qApp->setStyleSheet(qss);
        qssFile.close();
    }

    AVMainWnd w;
    w.show();

    return a.exec();
}
