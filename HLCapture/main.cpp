#include "hlcapture.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HLCapture w;
    w.show();
    return a.exec();
}
