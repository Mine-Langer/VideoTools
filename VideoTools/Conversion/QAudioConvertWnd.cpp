#include "QAudioConvertWnd.h"
#include "OptionSelectWnd.h"
#include "qvideoinfo.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

QAudioConvertWnd::QAudioConvertWnd(QWidget *parent)
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

QAudioConvertWnd::~QAudioConvertWnd()
{}

bool QAudioConvertWnd::DemuxPacket(AVPacket* pkt, int type)
{
	if (type == AVMEDIA_TYPE_AUDIO) {
		m_audioDecoder.SendPacket(pkt);
	}
	return true;
}

void QAudioConvertWnd::CleanPacket()
{

}

bool QAudioConvertWnd::VideoEvent(AVFrame* frame)
{
	m_remux.SendFrame(frame, AVMEDIA_TYPE_VIDEO);

	return true;
}

bool QAudioConvertWnd::AudioEvent(AVFrame* frame)
{
	m_remux.SendFrame(frame, AVMEDIA_TYPE_AUDIO);
	return true;
}

void QAudioConvertWnd::RemuxEvent(int nType)
{
	emit CvtStatusSig(nType);
}

void QAudioConvertWnd::InitView()
{
	QString DesktopLocation = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	ui.lineEditOutputPath->setText(DesktopLocation);
}

void QAudioConvertWnd::ConvertThread()
{
	QString szOutputPath = ui.lineEditOutputPath->text();
	for (int i = 0; i < m_vecFiles.size(); i++)
	{
		std::string szFile = m_vecFiles[i].toStdString();
		QFileInfo fileInfo(m_vecFiles[i]);
		QString fileName = fileInfo.fileName();
		fileName = fileName.left(fileName.indexOf(".") + 1);

		const char* cfilename = szFile.c_str();
		if (!m_demux.Open(cfilename))
			return;

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

		m_audioDecoder.Start(this);

		m_remux.Start(this);

		m_demux.WaitFinished();
		m_audioDecoder.WaitFinished();

		m_remux.WaitFinished();
	}

	emit CvtStatusSig(1);
}

void QAudioConvertWnd::OnBtnAddFileClick()
{
	QString	szTitle = "请选择音频文件";
	QString	szFilter = "音频文件(*.mp3 *.aac *.wav);;所有文件(*.*)";

	QString szFileName = QFileDialog::getOpenFileName(this, szTitle, "", szFilter);
	if (szFileName.isEmpty())
		return;

	QListWidgetItem* pItem = new QListWidgetItem(ui.ListItemWnd);
	QVideoInfo* videoInfo = new QVideoInfo(szFileName, ui.ListItemWnd);
	pItem->setSizeHint(QSize(300, 60));

	ui.ListItemWnd->setItemWidget(pItem, videoInfo);

	m_vecFiles.append(szFileName);
}

void QAudioConvertWnd::OnBtnAddPathClicked()
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

void QAudioConvertWnd::OnBtnStartClick()
{
	ui.btnStart->setEnabled(false);
	m_convertThread = std::thread(&QAudioConvertWnd::ConvertThread, this);
}

void QAudioConvertWnd::CvtStatusSlot(int ntype)
{
	if (ntype == 1)
	{
		ui.btnStart->setEnabled(true);
		QMessageBox::information(this, "格式转换完成", "提示", "确定");
	}
}

void QAudioConvertWnd::OnBtnChangeDirClicked()
{
	QString szPath = ui.lineEditOutputPath->text();
	QString szNewDir = QFileDialog::getExistingDirectory(this, "选择文件夹", szPath);
	ui.lineEditOutputPath->setText(szNewDir);
}

void QAudioConvertWnd::OnBtnOpenPathClicked()
{
	QString szPath = ui.lineEditOutputPath->text();
	QDesktopServices::openUrl(QUrl("file:" + szPath, QUrl::TolerantMode));
}

void QAudioConvertWnd::OnBtnOutputFmtClicked()
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
