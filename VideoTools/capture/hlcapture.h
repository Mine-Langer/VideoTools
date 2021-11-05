#pragma once
#pragma execution_character_set("utf-8")
#include <QtWidgets/QWidget>
#include "ui_hlcapture.h"
#include "CRecorder.h"

class CapturingDialog;
class HLCapture : public QWidget
{
    Q_OBJECT

public:
    HLCapture(QWidget *parent = Q_NULLPTR);

private: 
    void CreateCapWidget();

private slots:
    void OnRadioVideoClicked(bool);
    void OnBtnStartCaptureClicked();

private:
    Ui::HLCaptureClass ui;

    CapturingDialog* m_pCapWidget = nullptr;
    CRecorder m_recoder;
};
