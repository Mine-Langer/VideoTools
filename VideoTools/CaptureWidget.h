#pragma once
#include <QWidget>
#include <QTimer>
#include "Common.h"
#include "VideoRecorder.h"
#include "ui_CaptureView.h"

class QCaptureWidget : public QWidget
{
    Q_OBJECT

public:
    QCaptureWidget(QWidget *parent = Q_NULLPTR);


private:
	void Work();

    void CreateCapWidget();


private slots:
    void OnRadioVideoClicked(bool);
    void OnAudioTypeClicked(bool);
    void OnVisionTypeClicked(bool);
    void OnSaveTypeClicked(bool);
    void OnBtnStartCaptureClicked();
    void OnCaptureTimeout();

private:
    Ui::CaptureWidget ui;
    QTimer* m_timer = nullptr;
    CVideoRecorder m_VideoRecoder;

    class CapturingDialog* m_pCapWidget = nullptr;

    int m_videoType = 0; // ��Ƶѡ�� 0��ȫ��  1������
    // eAudioOpt m_audioType = NoAudio; // ��Ƶѡ�� 0����¼���� 1��ϵͳ���� 2����˷����� 3��ȫ��¼��
    int m_visionType = 0;// �������� 0������ 1������ 2��ԭ��
    int m_recordFmt = 0; // ¼�Ƹ�ʽ 0��MP4 1��FLV 2��AVI 3��Mp3 4��EXE

    bool m_bCapture = false; 
    unsigned int m_nCount = 0;
    std::thread m_thread;
};