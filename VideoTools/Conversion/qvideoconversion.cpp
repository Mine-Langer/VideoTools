#include "qvideoconversion.h"
#include "qvideoinfo.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QListWidget.h>

QVideoConversion::QVideoConversion(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.btnAddFile, SIGNAL(clicked()), this, SLOT(OnBtnAddFileClick()));
	connect(ui.btnStart, SIGNAL(clicked()), this, SLOT(OnBtnStartClick()));
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
	m_remux.SendFrame(frame, AVMEDIA_TYPE_VIDEO);

	return true;
}

bool QVideoConversion::AudioEvent(AVFrame* frame)
{
	m_remux.SendFrame(frame, AVMEDIA_TYPE_AUDIO);
	return true;
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

		m_demux.Start(this);

		if (m_videoDecoder.Open(&m_demux))
			m_videoDecoder.Start(this);

		if (m_audioDecoder.Open(&m_demux))
			m_audioDecoder.Start(this);

		int sample_rate, nb_sample;
		AVChannelLayout ch_layout; 
		AVSampleFormat sample_fmt;
		m_audioDecoder.GetSrcParameter(sample_rate, nb_sample, ch_layout, sample_fmt);
		std::string strName = m_szOutName.toStdString();
		if (!m_remux.SetOutput(strName.c_str(), m_nOutWidth, m_nOutHeight, ch_layout, sample_fmt, sample_rate))
			return;

		m_remux.Start();
	}	
}