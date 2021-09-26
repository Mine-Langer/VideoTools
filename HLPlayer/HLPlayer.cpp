#include "HLPlayer.h"
#include <QMouseEvent>

HLPlayer::HLPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    

	ui.sliderPlay->installEventFilter(this);
	ui.sliderVolumn->installEventFilter(this);
	connect(ui.btnPlay, SIGNAL(clicked()), this, SLOT(OnBtnPlayClicked()));
}

void HLPlayer::showEvent(QShowEvent* event)
{
	static bool bShow = false;
	if (!bShow)
	{
		bShow = true;
		if (m_player.Open("D:/OneDrive/video/ͬ����������δ��.mp4"))
		{
			m_player.InitWindow((const void*)ui.PlayView->winId(), ui.PlayView->width(), ui.PlayView->height());
			m_player.Start(this);
			m_bPlay = true;
		}
	}
}

void HLPlayer::resizeEvent(QResizeEvent* event)
{
	int width = ui.PlayView->width(); 
	int height = ui.PlayView->height();
}

bool HLPlayer::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui.sliderPlay || watched == ui.sliderVolumn)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() == Qt::LeftButton)
			{
				QSlider* pSlider = (QSlider*)watched;
				int dur = pSlider->maximum() - pSlider->minimum();
				int pos = pSlider->minimum() + dur * ((double)mouseEvent->x() / pSlider->width());
				if (pos != pSlider->sliderPosition())
					pSlider->setValue(pos);
			}
		}
	}
	return QObject::eventFilter(watched, event);
}

void HLPlayer::UpdateDuration(double dur)
{
	int64_t duration = dur;
	int h = duration / 3600;
	int m = (duration - (3600 * h)) / 60;
	int s = (duration - (3600 * h)) % 60;
	ui.labelEnd->setText(QString("%1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')));
	ui.sliderPlay->setRange(0, duration);
}

void HLPlayer::UpdatePlayPosition(double postion)
{
	int64_t duration = postion;
	int h = duration / 3600;
	int m = (duration - (3600 * h)) / 60;
	int s = (duration - (3600 * h)) % 60;
	ui.labelStart->setText(QString("%1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')));
	ui.sliderPlay->setValue(duration);
}

void HLPlayer::OnBtnPlayClicked()
{
	m_bPlay = !m_bPlay;
	if (m_bPlay)
	{
		ui.btnPlay->setStyleSheet(tr("QPushButton{ border-image: url(:/HLPlayer/res/play_normal.png); } \
								  QPushButton:hover { border-image: url(:/HLPlayer/res/play_hover.png); }\
								  QPushButton:pressed { border-image: url(:/HLPlayer/res/play_clicked.png);}"));
	}
	else
	{
		ui.btnPlay->setStyleSheet(tr("QPushButton{ border-image: url(:/HLPlayer/res/pause_normal.png); } \
								  QPushButton:hover { border-image: url(:/HLPlayer/res/pause_hover.png); }\
								  QPushButton:pressed { border-image: url(:/HLPlayer/res/pause_clicked.png);}"));
	}
}
