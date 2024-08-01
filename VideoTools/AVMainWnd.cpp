#include "AVMainWnd.h"
#include "CaptureWidget.h"
#include "CompositeView.h"
#include "FormatConversion.h"
#include "AVTools.h"
#include "VideoCutupWnd.h"

AVMainWnd::AVMainWnd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowIcon(QIcon(":/HLCapture/res/logo.png"));

	// 初始化系统参数
	AVTools::InitConfig();

	connect(ui.ListFunctionsWnd, &QListWidget::currentItemChanged, this, &AVMainWnd::OnFunctionsItemChanged);

	m_capture = new QCaptureWidget(this);
	m_compositeView = new CompositeView(this);
	m_conversion = new QFormatConversion(this, TVideo);
	m_audioConvert = new QFormatConversion(this, TAudio);
	m_videoCutup = new QFormatConversion(this);
	m_videoWatermark = new QFormatConversion(this);
	m_videoMerge = new QFormatConversion(this);
	m_videoToGIF = new QFormatConversion(this);

	ui.stackedWidget->addWidget(m_conversion);
	ui.stackedWidget->addWidget(m_audioConvert);
	ui.stackedWidget->addWidget(m_videoCutup);
	ui.stackedWidget->addWidget(m_capture);
	ui.stackedWidget->addWidget(m_compositeView);
	ui.stackedWidget->addWidget(m_videoWatermark);
	ui.stackedWidget->addWidget(m_videoMerge);
	ui.stackedWidget->addWidget(m_videoToGIF);
}

AVMainWnd::~AVMainWnd()
{
	delete m_capture;
	delete m_compositeView;
	delete m_conversion;
	delete m_audioConvert;
	delete m_videoCutup;
}


void AVMainWnd::OnFunctionsItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	int row = ui.ListFunctionsWnd->row(current);
	ui.labelTitle->setText(current->text());
	ui.stackedWidget->setCurrentIndex(row);
}
