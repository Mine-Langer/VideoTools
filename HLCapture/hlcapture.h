#pragma once

#include <QtWidgets/QWidget>
#include "ui_hlcapture.h"

class CaptureViewWidget;
class HLCapture : public QWidget
{
    Q_OBJECT

public:
    HLCapture(QWidget *parent = Q_NULLPTR);

private: 
    void CreateCapWidget();

private slots:
    void OnRadioVideoClicked(bool);

private:
    Ui::HLCaptureClass ui;

    CaptureViewWidget* m_pCapWidget = nullptr;
};
