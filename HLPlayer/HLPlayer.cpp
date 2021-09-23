#include "HLPlayer.h"

HLPlayer::HLPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    
   
}

void HLPlayer::showEvent(QShowEvent* event)
{
	static bool bShow = false;
	if (!bShow)
	{
		bShow = true;
		if (m_player.Open("D:/OneDrive/video/同胞布甲手足未必.mp4"))
		{
			m_player.InitWindow((const void*)ui.PlayView->winId(), ui.PlayView->width(), ui.PlayView->height());
			m_player.Start();
		}
	}
}

void HLPlayer::resizeEvent(QResizeEvent* event)
{
	int width = ui.PlayView->width(); 
	int height = ui.PlayView->height();
}
