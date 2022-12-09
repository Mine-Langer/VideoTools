#include <QFileDialog>
#include "CompositeView.h"
#include "ui_CompositeView.h"


CompositeView::CompositeView(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::CompositeView();
	ui->setupUi(this);

	m_imageMenu = new QMenu(ui->imageCtrlWidget);
	m_audioMenu = new QMenu(ui->audioCtrlWidget);
	QAction* actImage = new QAction(tr("插入图像"), m_imageMenu);
	QAction* actAudio = new QAction(tr("插入音频"), m_audioMenu);
	m_imageMenu->addAction(actImage);
	m_audioMenu->addAction(actAudio);

	// 默认时长3分钟
	const int defaultSecond = 180;
	ui->audioCtrlWidget->SetTimeLength(defaultSecond);
	ui->imageCtrlWidget->SetTimeLength(defaultSecond);
	ui->sliderTime->setRange(0, defaultSecond);

	connect(actImage, SIGNAL(triggered()), this, SLOT(OnActImage()));
	connect(actAudio, SIGNAL(triggered()), this, SLOT(OnActAudio()));
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
		m_composite.InitWnd((void*)ui->playView->winId(), ui->playView->width(), ui->playView->height());
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

	QByteArray arrFile = filename.toLocal8Bit();
	const char* szName = arrFile.data();

// 	QPixmap pixmap(filename);
// 	float ratio = (pixmap.height() * 1.00f) / (ui->imageCtrlWidget->height() * 1.00f);
// 	int calcWidth = (pixmap.width() * 1.00f) / ratio;
// 	ui->thumbnail_widget->setFixedSize(calcWidth, ui->imageCtrlWidget->height());
// 	ui->thumbnail_widget->setStyleSheet(QString(tr("border-image:url(%1);background-size:contain")).arg(filename));

	m_composite.AddImage(szName);
	m_comType |= 0x1;
	ui->imageCtrlWidget->AddTexture(filename);
}

void CompositeView::OnActAudio()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("音频文件(*.wav *.mp3 *.wma)"));
	if (filename.isEmpty())
		return;

	QByteArray arrFile = filename.toLocal8Bit();
	const char* szName = arrFile.data();

	m_composite.AddAudio(szName);
	m_comType |= 0x2;
	/*strcpy_s(m_szAudio, FILENAME_MAX, szName);

	if (m_player.OpenAudio(szName))
	{
		int duration = m_player.GetAudioDuration();
		int h = duration / 3600;
		int m = (duration - (3600 * h)) / 60;
		int s = (duration - (3600 * h)) % 60;
		ui->label_end_time->setText(QString("%1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')));
		ui->play_progress_slider->setRange(0, duration);
	}*/
}

void CompositeView::OnBtnPlay()
{
#ifdef TEST
	if (!m_player.Open("D:/documents/OneDrive/video/QQ视频搞钱.mp4")) //F:/MyDocuments/OneDrive/video D:/documents/OneDrive/video
		return;
	int width = ui->playView->width();
	int height = ui->playView->height();
	HWND hWnd = (HWND)ui->playView->winId();

	m_player.SetView(hWnd, width, height);

	m_player.Start();
#else
	 m_composite.Play();
#endif
}

void CompositeView::OnBtnExport()
{
	m_composite.SaveFile("output.mp4", m_comType);
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
