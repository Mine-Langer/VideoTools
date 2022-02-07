#include "HLPlayer.h"
#include <QtWidgets/QApplication>

#undef main
int main(int argc, char *argv[])
{
    float diff = pow(1.10, 1.0 / 6.0) - 1;
    QApplication a(argc, argv);
    HLPlayer w;
    w.show();
    return a.exec();
}
