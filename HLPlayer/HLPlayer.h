#pragma once
#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include "ui_HLPlayer.h"
#include "CPlayer.h"

class HLPlayer : public QWidget//, public IPlayEvent
{
    Q_OBJECT

public:
    HLPlayer(QWidget *parent = Q_NULLPTR);

private:
	void UpdateDuration(double duration) override;
	void UpdatePlayPosition(double postion) override;

private:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void OnBtnPlayClicked();
    void OnSliderPlayMoved(int);
    void OnBtnOpenFile();

private:
    Ui::HLPlayerClass ui;

    CPlayer m_player;
    bool m_bPlay = false;
};
