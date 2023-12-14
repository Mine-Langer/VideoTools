#include "qvideoconversion.h"
#include "OptionSelectWnd.h"
#include "qvideoinfo.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QListWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDebug>

QVideoConversion::QVideoConversion(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	InitView();


	connect(ui.btnAddFile, SIGNAL(clicked()), this, SLOT(OnBtnAddFileClick()));
	connect(ui.btnAddPath, SIGNAL(clicked()), this, SLOT(OnBtnAddPathClicked()));
	connect(ui.btnStart, SIGNAL(clicked()), this, SLOT(OnBtnStartClick()));
	connect(ui.btnChangeDir, SIGNAL(clicked()), this, SLOT(OnBtnChangeDirClicked()));
	connect(ui.btnOpenPath, SIGNAL(clicked()), this, SLOT(OnBtnOpenPathClicked()));
	connect(ui.btnOuputFormat, SIGNAL(clicked()), this, SLOT(OnBtnOutputFmtClicked()));
	connect(this, SIGNAL(CvtStatusSig(int)), this, SLOT(CvtStatusSlot(int)));
}

QVideoConversion::~QVideoConversion()
{
}


void QVideoConversion::setAVType(AVType type)
{
	m_avType = type;
}

void QVideoConversion::InitView()
{
	QString DesktopLocation = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	ui.lineEditOutputPath->setText(DesktopLocation);
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

void QVideoConversion::CleanPacket()
{
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

void QVideoConversion::RemuxEvent(int nType)
{
	emit CvtStatusSig(nType);
}


void QVideoConversion::OnBtnAddFileClick()
{
	QString szTitle, szFilter;
	if (m_avType == TAudio) {
		szTitle = "请选择音频文件";
		szFilter = "音频文件(*.mp3 *.aac *.wav);;所有文件(*.*)";
	}
	else if (m_avType == TVideo) {
		szTitle = "请选择视频文件";
		szFilter = "All Format (*.swf *.webm *.avi *.mp4 *.mov *.flv *.mpeg *.rm *.rmvb *.asf *.f4v *.mkv *.wmv)";
	}

	QString szFileName = QFileDialog::getOpenFileName(this, szTitle, "", szFilter);
	if (szFileName.isEmpty())
		return;

	QListWidgetItem* pItem = new QListWidgetItem(ui.videoListWnd);
	QVideoInfo* videoInfo = new QVideoInfo(szFileName, ui.videoListWnd);
	pItem->setSizeHint(QSize(300, 80));

	ui.videoListWnd->setItemWidget(pItem, videoInfo);

	m_vecFiles.append(szFileName);
}

void QVideoConversion::OnBtnAddPathClicked()
{
	QString szPath = QFileDialog::getExistingDirectory(this, "选择文件夹");
	QDir dir(szPath);
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QFileInfoList fileInfos = dir.entryInfoList();

	foreach(const QFileInfo & fileInfo, fileInfos) {
		QString szSuffix = fileInfo.suffix();
		QString filePath = fileInfo.absoluteFilePath();
	}
}

void QVideoConversion::OnBtnStartClick()
{
	QString szOutputPath = ui.lineEditOutputPath->text();
	for (int i = 0; i < m_vecFiles.size(); i++)
	{
		std::string szFile = m_vecFiles[i].toStdString();
		QFileInfo fileInfo(m_vecFiles[i]);
		QString fileName = fileInfo.fileName();
		fileName = fileName.left(fileName.indexOf(".")+1);
		
		const char* cfilename = szFile.c_str();
		if (!m_demux.Open(cfilename))
			return;

		if (m_avType == TVideo)
		{
			int srcWidth, srcHeight;
			AVPixelFormat srcFormat;
			if (m_videoDecoder.Open(&m_demux))
			{
				m_videoDecoder.GetSrcParameter(srcWidth, srcHeight, srcFormat);
				m_videoDecoder.SetSwsConfig();
			}
		}

		int sample_rate = 0;
		AVChannelLayout ch_layout = {};
		AVSampleFormat sample_fmt = AV_SAMPLE_FMT_NONE;
		if (m_audioDecoder.Open(&m_demux))
		{			
			m_audioDecoder.GetSrcParameter(sample_rate, ch_layout, sample_fmt);
			m_audioDecoder.SetSwrContext(ch_layout, sample_fmt, sample_rate);
		}
				
		std::string strName = (szOutputPath + "\\" + fileName + m_szOutputSuffix).toStdString();
		if (!m_remux.SetOutput(strName.c_str(), m_nOutWidth, m_nOutHeight, ch_layout, sample_fmt, sample_rate, m_outputBitRate))
			return;
		
		m_remux.SetType(m_avType);

		m_demux.Start(this);

		if (m_avType == TVideo)
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

void QVideoConversion::OnBtnChangeDirClicked()
{
	QString szPath = ui.lineEditOutputPath->text();
	QString szNewDir = QFileDialog::getExistingDirectory(this, "选择文件夹", szPath);
	ui.lineEditOutputPath->setText(szNewDir);
}

void QVideoConversion::OnBtnOpenPathClicked()
{
	QString szPath = ui.lineEditOutputPath->text();
	QDesktopServices::openUrl(QUrl("file:" + szPath, QUrl::TolerantMode));
}

void QVideoConversion::OnBtnOutputFmtClicked()
{
	OptionSelectWnd* optWnd = new OptionSelectWnd(this);
	QPoint pt = ui.btnOuputFormat->mapToGlobal(QPoint(0, 0));
	optWnd->setType(m_avType);
	optWnd->show();
	pt.setY(pt.y() - optWnd->geometry().height());
	//qDebug() << pt << " rect:"<<optWnd->geometry();
	optWnd->move(pt);
	connect(optWnd, &OptionSelectWnd::OptSelected, [this](QString szFormat, QString szValue) 
		{
			m_szOutputSuffix = szFormat;
			ui.btnOuputFormat->setText(szFormat + " " + szValue);
		});
}
