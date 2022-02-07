#include "HLPlayer.h"
#include <QMouseEvent>
#include <QFileDialog>
#include <QSlider>

HLPlayer::HLPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    
	ui.sliderPlay->installEventFilter(this);
	ui.sliderVolumn->installEventFilter(this);
	connect(ui.btnPlay, SIGNAL(clicked()), this, SLOT(OnBtnPlayClicked()));
	connect(ui.btnOpenFile, SIGNAL(clicked()), this, SLOT(OnBtnOpenFile()));
	connect(ui.sliderPlay, SIGNAL(sliderMoved(int)), this, SLOT(OnSliderPlayMoved(int)));
	connect(this, SIGNAL(PlayStatus(int)), this, SLOT(OnPlayerStatus(int)));
}

void HLPlayer::showEvent(QShowEvent* event)
{
	if (!m_bShow)
	{
		m_bShow = true;
		m_player.InitWindow((const void*)ui.PlayView->winId(), ui.PlayView->width(), ui.PlayView->height());
	}
}

void HLPlayer::resizeEvent(QResizeEvent* event)
{
	int width = ui.PlayView->width();
	int height = ui.PlayView->height();

	if (!m_bShow)
		return;

	m_player.UpdateWindow(width, height);
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

void HLPlayer::OnPlayStatus(eAVStatus eStatus)
{
	emit PlayStatus(eStatus);
}

void HLPlayer::closeEvent(QCloseEvent* event)
{
	m_player.Close();
}

// void HLPlayer::UpdateDuration(double dur)
// {
// 	int64_t duration = dur;
// 	int h = duration / 3600;
// 	int m = (duration - (3600 * h)) / 60;
// 	int s = (duration - (3600 * h)) % 60;
// 	ui.labelEnd->setText(QString("%1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')));
// 	ui.sliderPlay->setRange(0, duration);
// }
// 
// void HLPlayer::UpdatePlayPosition(double postion)
// {
// 	int64_t duration = postion;
// 	int h = duration / 3600;
// 	int m = (duration - (3600 * h)) / 60;
// 	int s = (duration - (3600 * h)) % 60;
// 	ui.labelStart->setText(QString("%1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')));
// 	ui.sliderPlay->setValue(duration);
// }

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

void HLPlayer::OnSliderPlayMoved(int value)
{

	return;
}

void HLPlayer::OnBtnOpenFile()
{
	QStringList filters;
	filters << tr("常见媒体格式 (*.avi *.mp4 *.flv *.mp3 *.wav *.wma)")
		<< tr("Windows Media 视频 (*.avi *.wmv *.wmp *.asf)")
		<< tr("Windows Media 音频 (*.wav *.wma *.mid *.midi)")
		<< tr("所有文件 (*.*)");
	QFileDialog fileDlg(this);
	fileDlg.setWindowTitle(tr("打开文件"));
	fileDlg.setNameFilters(filters);
	if (fileDlg.exec())
	{
		QStringList strList = fileDlg.selectedFiles();
		QByteArray baName = strList[0].toLocal8Bit();
		char* pszName = baName.data();

		m_player.UpdateWindow(0, 0, ui.PlayView->width(), ui.PlayView->height());
		if (m_player.Open(pszName))
		{
			m_player.Start(this);
			m_bPlay = true;
			ui.btnOpenFile->setVisible(false);
		}
	}
}

void HLPlayer::OnPlayerStatus(int iStatus)
{
	if (iStatus == eStop)
	{
		ui.btnOpenFile->setVisible(true);
		ui.PlayView->update();
	}
}
