#include "AVMainWnd.h"

AVMainWnd::AVMainWnd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.btnScreenRecording, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
	connect(ui.btnVideoComposite, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
	connect(ui.btnVideoTrans, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
	connect(ui.btnAudioTrans, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
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
		ui.labelTitle->setText("��Ļ¼��");
		layout->addWidget(&m_capture);
		m_pCurrentWidget = &m_capture;
	}
	else if (btnMenu == ui.btnVideoComposite)
	{
		ui.labelTitle->setText("��Ƶ�ϳ�");
		layout->addWidget(&m_compositeView);
		m_pCurrentWidget = &m_compositeView;
	}
	else if (btnMenu == ui.btnVideoTrans)
	{
		ui.labelTitle->setText("��Ƶת��");
		layout->addWidget(&m_conversion);
		m_pCurrentWidget = &m_conversion;
	}
	else if (btnMenu == ui.btnAudioTrans)
	{
		ui.labelTitle->setText("��Ƶת��");
		layout->addWidget(&m_conversion);
		m_pCurrentWidget = &m_conversion;
	}
	m_pCurrentWidget->setVisible(true);
}
