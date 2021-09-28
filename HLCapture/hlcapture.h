#pragma once

#include <QtWidgets/QWidget>
#include "ui_hlcapture.h"

class HLCapture : public QWidget
{
    Q_OBJECT

public:
    HLCapture(QWidget *parent = Q_NULLPTR);

private:
    Ui::HLCaptureClass ui;
};
