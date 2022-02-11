#pragma once
#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include <QTimer>
#include "ui_HLPlayer.h"
#include "CPlayer.h"

class HLPlayer : public QWidget, public IPlayerEvent
{
    Q_OBJECT

public:
    HLPlayer(QWidget *parent = Q_NULLPTR);

private:
//	void UpdateDuration(double duration) override;
//	void UpdatePlayPosition(double postion) override;

private:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void OnPlayStatus(eAVStatus eStatus) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void OnBtnPlayClicked();
    void OnBtnOpenFile();
    void OnPlayerStatus(int iStatus);
    void OnTestTimerout();

signals:
    void PlayStatus(int iStatus);

private:
    Ui::HLPlayerClass ui;

    CPlayer m_player;
    bool m_bPlay = false;
    bool m_bShow = false;

    // ceshi del
    QTimer* m_timer = nullptr;
    int m_nPos = 0;
};
