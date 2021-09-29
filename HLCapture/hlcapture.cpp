#include "hlcapture.h"
#include <QStandardPaths>

HLCapture::HLCapture(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.radioScreenCap->setChecked(true);
    ui.radioAudioSys->setChecked(true);
    ui.radioHighQuality->setChecked(true);
    ui.radioMP4->setChecked(true);
    QString szDesktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    ui.editOutputPath->setText(szDesktopPath);
}
