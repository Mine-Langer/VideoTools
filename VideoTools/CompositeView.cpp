#include "Common.h"
#include "CompositeView.h"
#include <QFileDialog>
#include "ui_CompositeView.h"


CompositeView::CompositeView(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::CompositeView();
	ui->setupUi(this);

	m_imageMenu = new QMenu(ui->imageCtrlWidget);
	m_audioMenu = new QMenu(ui->audioCtrlWidget);
	QAction* actImage = new QAction(tr("插入图像"), m_imageMenu);
	QAction* actImageDel = new QAction(tr("删除图像"), m_imageMenu);
	QAction* actAudio = new QAction(tr("插入音频"), m_audioMenu);
	QAction* actAudioDel = new QAction(tr("删除音频"), m_imageMenu);
	m_imageMenu->addAction(actImage);
	m_audioMenu->addAction(actAudio);

	// 默认时长3分钟
	const int defaultSecond = 180;
	ui->audioCtrlWidget->SetTimeLength(defaultSecond);
	ui->imageCtrlWidget->SetTimeLength(defaultSecond);
	ui->sliderTime->setRange(0, defaultSecond);

	connect(actImage, SIGNAL(triggered()), this, SLOT(OnActImage()));
	connect(actAudio, SIGNAL(triggered()), this, SLOT(OnActAudio()));
	connect(actImageDel, SIGNAL(triggered()), this, SLOT(OnActImageDel()));
	connect(actAudioDel, SIGNAL(triggered()), this, SLOT(OnActAudioDel()));
	connect(ui->imageCtrlWidget, &AVTextureBar::customContextMenuRequested, this, &CompositeView::OnImageWidgetContextMenuRequested);
	connect(ui->audioCtrlWidget, &AVTextureBar::customContextMenuRequested, this, &CompositeView::OnAudioWidgetContextMenuRequested);
	connect(ui->btnPlay, &QPushButton::clicked, this, &CompositeView::OnBtnPlay);
	connect(ui->btnExport, &QPushButton::clicked, this, &CompositeView::OnBtnExport);
	connect(ui->timeDuration, &QTimeEdit::timeChanged, this, &CompositeView::timeDurationChanged);
	connect(ui->sliderTime, &QSlider::sliderMoved, this, &CompositeView::sliderTimeMoved);
}

CompositeView::~CompositeView()
{
	delete ui;
}


void CompositeView::showEvent(QShowEvent* event)
{
	static bool bShow = true;
	if (bShow)
	{
		bShow = false;
		HWND hWnd = (HWND)ui->playView->winId();
	//	m_composite.InitWnd(hWnd, ui->playView->width(), ui->playView->height());
	}
}

void CompositeView::OnImageWidgetContextMenuRequested(const QPoint& pos)
{
	ui->imageCtrlWidget->UpdatePostion();
	m_imageMenu->exec(QCursor::pos());
}

void CompositeView::OnAudioWidgetContextMenuRequested(const QPoint& pos)
{
	m_audioMenu->exec(QCursor::pos());
}

void CompositeView::OnActImage()
{	
	QString filename = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("图像文件(*.jpg *.bmp *.png);;  所有文件(*.*)"));
	if (filename.isEmpty())
		return;

	ui->imageCtrlWidget->AddTexture(filename, 1);

	m_videoSynthesis.AddImage(filename.toStdString());
}

void CompositeView::OnActAudio()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("音频文件(*.wav *.mp3 *.wma)"));
	if (filename.isEmpty())
		return;

	ui->audioCtrlWidget->AddTexture(filename, 2);

	m_videoSynthesis.AddAudio(filename.toStdString());
}

void CompositeView::OnActImageDel()
{

}

void CompositeView::OnActAudioDel()
{

}

void CompositeView::OnBtnPlay()
{
	int width = ui->playView->width();
	int height = ui->playView->height();
	HWND hWnd = (HWND)ui->playView->winId();

//	if (!m_player.Open("D:/documents/OneDrive/video/QQ视频搞钱.mp4")) //F:/MyDocuments/OneDrive/video D:/documents/OneDrive/video
//		return;
	

//	m_player.SetView(hWnd/*, width, height*/);

//	m_player.Start();
//	m_player.SetAudioSpec();
	m_videoSynthesis.BindShowWindow(hWnd, width, height);
	m_videoSynthesis.Start();

//	std::vector<ItemElem> vecAudio = ui->audioCtrlWidget->GetItemList();
//	std::vector<ItemElem> vecImage = ui->imageCtrlWidget->GetItemList();

	//m_composite.InitWnd(hWnd, width, height);

	//m_composite.Play(vecImage, vecAudio);
}

void CompositeView::OnBtnExport()
{
	std::vector<ItemElem> vecAudio = ui->audioCtrlWidget->GetItemList();
	std::vector<ItemElem> vecImage = ui->imageCtrlWidget->GetItemList();
	//m_composite.SaveFile("output.mp4", vecImage, vecAudio);
	m_comType = 0;
}

void CompositeView::timeDurationChanged(const QTime& time)
{
	QString szText = time.toString("mm:ss");
	int secDuration = time.msecsSinceStartOfDay() / 1000;
	ui->sliderTime->setRange(0, secDuration);
	ui->labelEndTime->setText(szText);
	ui->audioCtrlWidget->SetTimeLength(secDuration);
	ui->imageCtrlWidget->SetTimeLength(secDuration);
}

void CompositeView::sliderTimeMoved(int position)
{
	QString szText = QString("%1:%2").arg(position / 60, 2, 10, QLatin1Char('0')).arg(position % 60, 2, 10, QLatin1Char('0'));
	ui->labelStartTime->setText(szText);
}

