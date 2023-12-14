#include "AVMainWnd.h"

AVMainWnd::AVMainWnd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.btnScreenRecording, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
	connect(ui.btnVideoComposite, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
	connect(ui.btnVideoTrans, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
	connect(ui.btnAudioTrans, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));

	m_capture = new HLCapture(this);
	m_compositeView = new CompositeView(this);
	m_conversion = new QVideoConversion(this);
	m_capture->setVisible(false);
	m_compositeView->setVisible(false);
	m_conversion->setVisible(false);
}

AVMainWnd::~AVMainWnd()
{
}

void AVMainWnd::OnBtnMenuClicked()
{
	QPushButton* btnMenu = qobject_cast<QPushButton*>(sender());
	QLayout* layout = ui.mainPageView->layout();
	if (m_pCurrentWidget != nullptr) {
		m_pCurrentWidget->setVisible(false);
		layout->removeWidget(m_pCurrentWidget);
	}

	if (btnMenu == ui.btnScreenRecording)
	{
		ui.labelTitle->setText("ÆÁÄ»Â¼Ïñ");
		layout->addWidget(m_capture);
		m_pCurrentWidget = m_capture;
	}
	else if (btnMenu == ui.btnVideoComposite)
	{
		ui.labelTitle->setText("ÊÓÆµºÏ³É");
		layout->addWidget(m_compositeView);
		m_pCurrentWidget = m_compositeView;
	}
	else if (btnMenu == ui.btnVideoTrans)
	{
		ui.labelTitle->setText("ÊÓÆµ×ª»»");
		layout->addWidget(m_conversion);
		m_conversion->setAVType(TVideo);
		m_pCurrentWidget = m_conversion;
	}
	else if (btnMenu == ui.btnAudioTrans)
	{
		ui.labelTitle->setText("ÒôÆµ×ª»»");
		layout->addWidget(m_conversion);
		m_conversion->setAVType(TAudio);
		m_pCurrentWidget = m_conversion;
	}
	m_pCurrentWidget->setVisible(true);
}
