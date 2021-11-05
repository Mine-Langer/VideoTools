#include "hlcapture.h"
#include "CaptureViewWidget.h"
#include <QStandardPaths>
#include <QBitmap>
#include <QDateTime>

HLCapture::HLCapture(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.radioScreenCap->setChecked(true);
    ui.radioAudioSys->setChecked(true);
    ui.radioHighQuality->setChecked(true);
    ui.radioMP4->setChecked(true);
    QString szDesktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    ui.editOutputPath->setText(szDesktopPath);

    connect(ui.radioAreaCap, SIGNAL(clicked(bool)), this, SLOT(OnRadioVideoClicked(bool)));
    connect(ui.radioScreenCap, SIGNAL(clicked(bool)), this, SLOT(OnRadioVideoClicked(bool)));
    connect(ui.btnStartCap, SIGNAL(clicked()), this, SLOT(OnBtnStartCaptureClicked()));
}

void HLCapture::CreateCapWidget()
{
   
}

void HLCapture::OnRadioVideoClicked(bool bCheck)
{
    QRadioButton* radioBtn = qobject_cast<QRadioButton*>(sender());
    if (bCheck)
    {
        if (radioBtn == ui.radioAreaCap)
        {

        }
        else if (radioBtn == ui.radioScreenCap)
        {
            if (!m_pCapWidget)
            {
                m_pCapWidget = new CapturingDialog;
                m_pCapWidget->show();
            }
        }
    }
}

void HLCapture::OnBtnStartCaptureClicked()
{
    QDateTime currTime = QDateTime::currentDateTime();
    QString fileName = currTime.toString("yyyy-mm-dd hhmmsszzz");
    QString szPath = ui.editOutputPath->text();
    QString szFilePath = szPath + "\\" + fileName + ".mp4";
    QByteArray baName = szFilePath.toLocal8Bit();
    const char* pszName = baName.data();
    QDesktopWidget* screen = QApplication::desktop();
	QRect mm = screen->screenGeometry();
	int screen_width = mm.width();
	int screen_height = mm.height();
    m_recoder.Init(0,0, screen_width, screen_height);
    m_recoder.Run(pszName);

}
