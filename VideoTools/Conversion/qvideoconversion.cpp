#include "qvideoconversion.h"
#include "qvideoinfo.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QListWidget>
#include <QMessageBox>

QVideoConversion::QVideoConversion(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.btnAddFile, SIGNAL(clicked()), this, SLOT(OnBtnAddFileClick()));
	connect(ui.btnStart, SIGNAL(clicked()), this, SLOT(OnBtnStartClick()));
	connect(this, SIGNAL(CvtStatusSig(int)), this, SLOT(CvtStatusSlot(int)));
}

QVideoConversion::~QVideoConversion()
{
}


bool QVideoConversion::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_VIDEO) {
		m_videoDecoder.SendPacket(pkt);
	}
	else if (type == AVMEDIA_TYPE_AUDIO) {
		m_audioDecoder.SendPacket(pkt);
	}
	return true;
}

bool QVideoConversion::VideoEvent(AVFrame* frame)
{
	AVFrame* cvtFrame = nullptr;
	if (frame)
		cvtFrame = m_videoDecoder.ConvertFrame(frame);

	m_remux.SendFrame(cvtFrame, AVMEDIA_TYPE_VIDEO);

	return true;
}

bool QVideoConversion::AudioEvent(AVFrame* frame)
{
	m_remux.SendFrame(frame, AVMEDIA_TYPE_AUDIO);
	return true;
}

void QVideoConversion::RemuxEvent(int nType)
{
	emit CvtStatusSig(nType);
}


void QVideoConversion::OnBtnAddFileClick()
{
	QString szFileName = QFileDialog::getOpenFileName(this, QStringLiteral("请选择视频文件"), "", "All Format (*.swf *.webm *.avi *.mp4 *.mov *.flv *.mpeg *.rm *.rmvb *.asf *.f4v *.mkv *.wmv)");
	if (szFileName.isEmpty())
		return;

	QListWidgetItem* pItem = new QListWidgetItem(ui.videoListWnd);
	QVideoInfo* videoInfo = new QVideoInfo(szFileName, ui.videoListWnd);
	pItem->setSizeHint(QSize(300, 160));

	ui.videoListWnd->setItemWidget(pItem, videoInfo);

	m_vecFiles.append(szFileName);
}

void QVideoConversion::OnBtnStartClick()
{
	for (int i = 0; i < m_vecFiles.size(); i++)
	{
		std::string szFile = m_vecFiles[i].toStdString();
		const char* cfilename = szFile.c_str();
		if (!m_demux.Open(cfilename))
			return;

		int srcWidth, srcHeight; 
		AVPixelFormat srcFormat;
		if (m_videoDecoder.Open(&m_demux))
		{
			m_videoDecoder.GetSrcParameter(srcWidth, srcHeight, srcFormat);
			m_videoDecoder.SetSwsConfig();
		}

		int sample_rate;
		AVChannelLayout ch_layout;
		AVSampleFormat sample_fmt;
		if (m_audioDecoder.Open(&m_demux))
		{			
			m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
			m_audioDecoder.SetSwrContext(ch_layout, AV_SAMPLE_FMT_S16, sample_rate);
		}
				
		std::string strName = m_szOutName.toStdString();
		if (!m_remux.SetOutput(strName.c_str(), m_nOutWidth, m_nOutHeight, ch_layout, sample_fmt, sample_rate))
			return;

		m_demux.Start(this);

		m_videoDecoder.Start(this);

		m_audioDecoder.Start(this);

		m_remux.Start(this);
	}	
}

void QVideoConversion::CvtStatusSlot(int ntype)
{
	if (ntype == 1)
		QMessageBox::information(this, QStringLiteral("格式转换完成"), QStringLiteral("提示"));
}
