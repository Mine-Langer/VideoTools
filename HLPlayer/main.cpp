#include "HLPlayer.h"
#include <QtWidgets/QApplication>

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HLPlayer w;
    w.show();
    return a.exec();
}
