#include "FormatConversion.h"
#include "OptionSelectWnd.h"
#include "qvideoinfo.h"
#include "AVTools.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QListWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDebug>

QFormatConversion::QFormatConversion(QWidget *parent, AVType avType)
	: QWidget(parent), m_avType(avType)
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

QFormatConversion::~QFormatConversion()
{
	Release();
}


void QFormatConversion::InitView()
{
	QString DesktopLocation = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	ui.lineEditOutputPath->setText(DesktopLocation);

	m_szOutputPath = DesktopLocation;
}

void QFormatConversion::AddFileToList(const QString& szFile)
{
	QListWidgetItem* pItem = new QListWidgetItem(ui.ListWnd);
	QVideoInfo* videoInfo = new QVideoInfo(szFile, ui.ListWnd);
	pItem->setSizeHint(QSize(300, 60));

	ui.ListWnd->setItemWidget(pItem, videoInfo);

	m_vecFiles.append(qMakePair(szFile, videoInfo));
}

void QFormatConversion::Release()
{
	if (m_convertThread.joinable())
		m_convertThread.join();
}

#if 0
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
#endif
void QFormatConversion::OnBtnAddFileClick()
{
	QString szTitle, szFilter;

	szTitle = m_avType == TVideo ? "请选择视频文件" : "请选择音频文件";
	QStringList VideoFmtList = AVTools::VideoFormatList();
	QStringList AudioFmtList = AVTools::AudioFormatList();
	QString szVideoFMT, szAudioFMT;
	for (QString szFmt : VideoFmtList) szVideoFMT += ("*." + szFmt.toLower() + " ");
	for (QString szFmt : AudioFmtList) szAudioFMT += ("*." + szFmt.toLower() + " ");
	szFilter = m_avType == TVideo ? ("视频文件 ("+szVideoFMT+");;") : ("音频文件 ("+ szAudioFMT+ ");;");
	szFilter += "所有文件 (*.*)";

	QString szFileName = QFileDialog::getOpenFileName(this, szTitle, "", szFilter);
	if (szFileName.isEmpty())
		return;

	AddFileToList(szFileName);
}

void QFormatConversion::OnBtnAddPathClicked()
{
	QString szPath = QFileDialog::getExistingDirectory(this, "选择文件夹");
	if (szPath.isEmpty())
		return;

	QDir dir(szPath);
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QFileInfoList fileInfos = dir.entryInfoList();

	QStringList FormatList = m_avType == TVideo ? AVTools::VideoFormatList() : AVTools::AudioFormatList();
	foreach(const QFileInfo & fileInfo, fileInfos) {
		QString szSuffix = fileInfo.suffix();
		QString filePath = fileInfo.absoluteFilePath();
		if (FormatList.contains(szSuffix, Qt::CaseInsensitive))
		{
			AddFileToList(filePath);
		}
	}
}

void QFormatConversion::OnBtnStartClick()
{
	ui.btnStart->setEnabled(false);
	ui.btnStart->setText("正在转换");
	m_convertThread = std::thread(&QFormatConversion::ConvertThread, this);
}

void QFormatConversion::CvtStatusSlot(int ntype)
{
	if (ntype == 1)
	{
		ui.btnStart->setEnabled(true);
		QMessageBox::information(this, "格式转换完成", "提示", "确定");
	}
}

void QFormatConversion::OnBtnChangeDirClicked()
{
	QString szPath = ui.lineEditOutputPath->text();
	QString szNewDir = QFileDialog::getExistingDirectory(this, "选择文件夹", szPath);
	ui.lineEditOutputPath->setText(szNewDir);
}

void QFormatConversion::OnBtnOpenPathClicked()
{
	QString szPath = ui.lineEditOutputPath->text();
	QDesktopServices::openUrl(QUrl("file:" + szPath, QUrl::TolerantMode));
}

void QFormatConversion::OnBtnOutputFmtClicked()
{
	OptionSelectWnd* optWnd = new OptionSelectWnd(this);
	QPoint pt = ui.btnOuputFormat->mapToGlobal(QPoint(0, 0));
	optWnd->setType(m_avType);
	optWnd->show();
	pt.setY(pt.y() - optWnd->geometry().height());

	optWnd->move(pt);
	connect(optWnd, &OptionSelectWnd::OptSelected, [this](QString szFormat, QString szValue, QString szParam) 
		{
			m_szOutputSuffix = szFormat;
			m_szOutputParam = szParam;
			ui.btnOuputFormat->setText(szFormat + " " + szValue);
		});
}

void QFormatConversion::ConvertThread()
{
	QString szOutputPath = ui.lineEditOutputPath->text();
	for (int i = 0; i < m_vecFiles.size(); i++)
	{
		QString szFile = m_vecFiles[i].first;
		QVideoInfo* pVideoInfo = m_vecFiles[i].second;
		QFileInfo fileInfo(szFile);
		QString fileName = fileInfo.fileName();
		fileName = fileName.left(fileName.indexOf(".") + 1);

		pVideoInfo->SetParam(m_szOutputSuffix, m_szOutputParam);
		pVideoInfo->SetSavePath(m_szOutputPath);
		pVideoInfo->Start();

#if 0
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

		m_remux.SetType(TAll);

		m_demux.Start(this);

		m_videoDecoder.Start(this);

		m_audioDecoder.Start(this);

		m_remux.Start(this);

		m_demux.WaitFinished();
		m_audioDecoder.WaitFinished();
		m_videoDecoder.WaitFinished();

		m_remux.WaitFinished();
#endif
	}

	emit CvtStatusSig(1);
}