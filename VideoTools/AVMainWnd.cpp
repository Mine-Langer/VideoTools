#include "AVMainWnd.h"
#include "capture/hlcapture.h"
#include "Composition/CompositeView.h"
#include "Conversion/qvideoconversion.h"
#include "Conversion/QAudioConvertWnd.h"

AVMainWnd::AVMainWnd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.ListFunctionsWnd, &QListWidget::currentItemChanged, this, &AVMainWnd::OnFunctionsItemChanged);

	m_capture = new HLCapture(this);
	m_compositeView = new CompositeView(this);
	m_conversion = new QVideoConversion(this);
	m_audioConvert = new QAudioConvertWnd(this);
	ui.stackedWidget->addWidget(m_conversion);
	ui.stackedWidget->addWidget(m_audioConvert);
	ui.stackedWidget->addWidget(m_capture);
	ui.stackedWidget->addWidget(m_compositeView);
}

AVMainWnd::~AVMainWnd()
{
}


void AVMainWnd::OnFunctionsItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	int row = ui.ListFunctionsWnd->row(current);
	ui.labelTitle->setText(current->text());
	ui.stackedWidget->setCurrentIndex(row);
}
