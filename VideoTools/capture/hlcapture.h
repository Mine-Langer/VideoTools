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

    int m_videoType = 0; // 视频选项 0：全屏  1：区域
    int m_audioType = 0; // 音频选项 0：不录声音 1：系统声音 2：麦克风声音 3：全部录制
    int m_visionType = 0;// 画质设置 0：标清 1：高清 2：原画
    int m_recordFmt = 0; // 录制格式 0：MP4 1：FLV 2：AVI 3：Mp3 4：EXE
};
