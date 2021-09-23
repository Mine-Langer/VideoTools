#pragma once

#include <QtWidgets/QWidget>
#include "ui_HLPlayer.h"
#include "CPlayer.h"

class HLPlayer : public QWidget
{
    Q_OBJECT

public:
    HLPlayer(QWidget *parent = Q_NULLPTR);

private:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::HLPlayerClass ui;

    CPlayer m_player;
};
