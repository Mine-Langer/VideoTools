#pragma once
#pragma execution_character_set("utf-8")
#include <QtWidgets/QWidget>
#include <QTimer>
#include "ui_hlcapture.h"
#include "CRecoder.h"

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
    void OnAudioTypeClicked(bool);
    void OnVisionTypeClicked(bool);
    void OnSaveTypeClicked(bool);
    void OnBtnStartCaptureClicked();
    void OnCaptureTimeout();

private:
    Ui::HLCaptureClass ui;
    QTimer* m_timer = nullptr;

    CapturingDialog* m_pCapWidget = nullptr;
    CRecoder m_recoder;

    int m_videoType = 0; // ��Ƶѡ�� 0��ȫ��  1������
    eAudioOpt m_audioType = NoAudio; // ��Ƶѡ�� 0����¼���� 1��ϵͳ���� 2����˷����� 3��ȫ��¼��
    int m_visionType = 0;// �������� 0������ 1������ 2��ԭ��
    int m_recordFmt = 0; // ¼�Ƹ�ʽ 0��MP4 1��FLV 2��AVI 3��Mp3 4��EXE

    bool m_bCapture = false; 
    unsigned int m_nCount = 0;
};
