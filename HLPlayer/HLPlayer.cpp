#include "HLPlayer.h"

HLPlayer::HLPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    
    if (m_player.Open("D:/OneDrive/video/同胞布甲手足未必.mp4"))
    {
        m_player.Start();
    }
}
