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
    void OnAudioTypeClicked(bool);
    void OnVisionTypeClicked(bool);
    void OnSaveTypeClicked(bool);
    void OnBtnStartCaptureClicked();

private:
    Ui::HLCaptureClass ui;

    CapturingDialog* m_pCapWidget = nullptr;
    CRecorder m_recoder;

    int m_videoType = 0; // ��Ƶѡ�� 0��ȫ��  1������
    int m_audioType = 0; // ��Ƶѡ�� 0����¼���� 1��ϵͳ���� 2����˷����� 3��ȫ��¼��
    int m_visionType = 0;// �������� 0������ 1������ 2��ԭ��
    int m_recordFmt = 0; // ¼�Ƹ�ʽ 0��MP4 1��FLV 2��AVI 3��Mp3 4��EXE
};
