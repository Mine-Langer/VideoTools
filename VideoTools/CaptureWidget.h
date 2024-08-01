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

    int m_videoType = 0; // 视频选项 0：全屏  1：区域
    // eAudioOpt m_audioType = NoAudio; // 音频选项 0：不录声音 1：系统声音 2：麦克风声音 3：全部录制
    int m_visionType = 0;// 画质设置 0：标清 1：高清 2：原画
    int m_recordFmt = 0; // 录制格式 0：MP4 1：FLV 2：AVI 3：Mp3 4：EXE

    bool m_bCapture = false; 
    unsigned int m_nCount = 0;
    std::thread m_thread;
};