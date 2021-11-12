#include "AVMainWnd.h"

AVMainWnd::AVMainWnd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.btnScreenRecording, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
	connect(ui.btnVideoComposite, SIGNAL(clicked()), this, SLOT(OnBtnMenuClicked()));
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
		layout->addWidget(&m_capture);
		m_pCurrentWidget = &m_capture;
	}

	if (btnMenu == ui.btnVideoComposite)
	{
		layout->addWidget(&m_compositeView);
		m_pCurrentWidget = &m_compositeView;
	}
	m_pCurrentWidget->setVisible(true);
}
