#include "HLPlayer.h"

HLPlayer::HLPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    
    if (m_player.Open("D:/OneDrive/video/ͬ����������δ��.mp4"))
    {
        m_player.Start();
    }
}
