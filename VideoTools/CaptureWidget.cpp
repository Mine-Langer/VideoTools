#include "CaptureWidget.h"
#include "CaptureViewWidget.h"
#include <QStandardPaths>
#include <QBitmap>
#include <QDateTime>
#include <QDesktopWidget>
#include "VideoRecorder.h"

QCaptureWidget::QCaptureWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.radioScreenCap->setChecked(true);
    ui.radioAudioSys->setChecked(true);
    ui.radioHighQuality->setChecked(true);
    ui.radioMP4->setChecked(true);
    QString szDesktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    ui.editOutputPath->setText(szDesktopPath);

	m_timer = new QTimer(this);
    connect(ui.radioAreaCap, SIGNAL(clicked(bool)), this, SLOT(OnRadioVideoClicked(bool)));
    connect(ui.radioScreenCap, SIGNAL(clicked(bool)), this, SLOT(OnRadioVideoClicked(bool)));

    connect(ui.radioAudioAll, SIGNAL(clicked(bool)), this, SLOT(OnAudioTypeClicked(bool)));
    connect(ui.radioAudioSys, SIGNAL(clicked(bool)), this, SLOT(OnAudioTypeClicked(bool)));
    connect(ui.radioAudioMic, SIGNAL(clicked(bool)), this, SLOT(OnAudioTypeClicked(bool)));
    connect(ui.radioAudioNoCap, SIGNAL(clicked(bool)), this, SLOT(OnAudioTypeClicked(bool)));

    connect(ui.radioStandardQuality, SIGNAL(clicked(bool)), this, SLOT(OnVisionTypeClicked(bool)));
    connect(ui.radioHighQuality, SIGNAL(clicked(bool)), this, SLOT(OnVisionTypeClicked(bool)));
    connect(ui.radiOriginalQuality, SIGNAL(clicked(bool)), this, SLOT(OnVisionTypeClicked(bool)));

    connect(ui.radioMP4, SIGNAL(clicked(bool)), this, SLOT(OnSaveTypeClicked(bool)));
    connect(ui.radioFLV, SIGNAL(clicked(bool)), this, SLOT(OnSaveTypeClicked(bool)));
    connect(ui.radioAVI, SIGNAL(clicked(bool)), this, SLOT(OnSaveTypeClicked(bool)));
    connect(ui.radioMP3, SIGNAL(clicked(bool)), this, SLOT(OnSaveTypeClicked(bool)));
    connect(ui.radioEXE, SIGNAL(clicked(bool)), this, SLOT(OnSaveTypeClicked(bool)));

    connect(ui.btnStartCap, SIGNAL(clicked()), this, SLOT(OnBtnStartCaptureClicked()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(OnCaptureTimeout()));
}


void QCaptureWidget::Work()
{
    while (m_bCapture)
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

void QCaptureWidget::CreateCapWidget()
{
   
}

void QCaptureWidget::OnRadioVideoClicked(bool bCheck)
{
    QRadioButton* radioBtn = qobject_cast<QRadioButton*>(sender());
    if (bCheck)
    {
#if 0
        if (radioBtn == ui.radioAreaCap)
        {
            if (m_pCapWidget)
                delete m_pCapWidget;
        }
        else if (radioBtn == ui.radioScreenCap)
        {
            if (!m_pCapWidget)
            {
                m_pCapWidget = new CapturingDialog;
                m_pCapWidget->show();
            }
        }
#endif
    }
}

void QCaptureWidget::OnAudioTypeClicked(bool)
{
    QRadioButton* pRadio = qobject_cast<QRadioButton*>(sender());
#if 0
    if (pRadio == ui.radioAudioAll)
        m_audioType = AllCap;
    if (pRadio == ui.radioAudioSys)
        m_audioType = SysAudio;
    if (pRadio == ui.radioAudioMic)
        m_audioType = MicroAudio;
    if (pRadio == ui.radioAudioNoCap)
        m_audioType = NoAudio;
#endif
}

void QCaptureWidget::OnVisionTypeClicked(bool)
{
    QRadioButton* pRadio = qobject_cast<QRadioButton*>(sender());
    if (pRadio == ui.radioStandardQuality)
        m_visionType = 0;
    if (pRadio == ui.radioHighQuality)
        m_visionType = 1;
	if (pRadio == ui.radiOriginalQuality)
        m_visionType = 2;
}

void QCaptureWidget::OnSaveTypeClicked(bool)
{
	QRadioButton* pRadio = qobject_cast<QRadioButton*>(sender());
	if (pRadio == ui.radioMP4)
        m_recordFmt = 0;
	if (pRadio == ui.radioFLV)
        m_recordFmt = 1;
	if (pRadio == ui.radioAVI)
        m_recordFmt = 2;
    if (pRadio == ui.radioMP3)
        m_recordFmt = 3;
    if (pRadio == ui.radioEXE)
        m_recordFmt = 4;
}

void QCaptureWidget::OnBtnStartCaptureClicked()
{
	int screen_width = 0, screen_height = 0;
	int posX = 0, posY = 0;
    QString szSuffic = m_recordFmt == 0 ? ".mp4" : 
        m_recordFmt == 1 ? ".flv" : 
        m_recordFmt == 2 ? ".avi" : 
        m_recordFmt == 3 ? ".mp3" : ".mp4";
       
    QDateTime currTime = QDateTime::currentDateTime();
    QString fileName = currTime.toString("yyyy-mm-dd hhmmsszzz");
    QString szPath = ui.editOutputPath->text();
    QString szFilePath = szPath + "\\" + fileName + szSuffic;
    QByteArray baName = szFilePath.toLocal8Bit();
    const char* pszName = baName.data();

	m_bCapture = !m_bCapture;

    if (m_videoType == 0) // È«ÆÁ
    {
		QDesktopWidget* screen = QApplication::desktop();
		QRect mm = screen->screenGeometry();
		screen_width = mm.width();
		screen_height = mm.height();
    }
    else if (m_videoType == 1)
    {
#if 0
        posX = m_pCapWidget->geometry().x();
        posY = m_pCapWidget->geometry().y();
        screen_width = m_pCapWidget->geometry().width();
        screen_height = m_pCapWidget->geometry().height();
#endif
    }
   
	if (m_bCapture)
	{
		m_nCount = 0;
		ui.labelCapDuration->setText("00:00:00");
		ui.btnStartCap->setText(tr("Í£Ö¹"));

		m_VideoRecoder.Init(2, screen_width, screen_height);
		m_VideoRecoder.Start();

        m_timer->start(1000);
	}
	else
	{
		ui.btnStartCap->setText(tr("¿ªÊ¼"));
		m_timer->stop();
        
        m_VideoRecoder.Stop();        
	} 
}

void QCaptureWidget::OnCaptureTimeout()
{
    m_nCount++;
    QString szCount = QString("%1:%2:%3").arg(m_nCount/3600, 2, 10, QLatin1Char('0')).arg(m_nCount%3600/60, 2, 10, QLatin1Char('0')).arg(m_nCount%60, 2, 10, QLatin1Char('0'));
    ui.labelCapDuration->setText(szCount);
}