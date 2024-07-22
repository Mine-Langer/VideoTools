#include "Common.h"
#include "qvideoinfo.h"
#include <QFileInfo>
#include <QPainter>
#include "ui_qvideoinfo.h"

QVideoInfo::QVideoInfo(const QString& szFile, QWidget *parent)
	: QWidget(parent), m_filename(szFile)
{
	ui = new Ui::QVideoInfo();
	ui->setupUi(this);
	
	Open(szFile);
}

QVideoInfo::~QVideoInfo()
{
	delete ui;
}

void QVideoInfo::SetSavePath(const QString& szPath)
{
	m_savePath = szPath;
}

void QVideoInfo::SetParam(const QString& szSuffix, const QString& szParam)
{
	if (!szParam.isEmpty())
	{
		QStringList params = szParam.split("x");
		m_outWith = params[0].toInt();
		m_outHeight = params[1].toInt();
	}
	m_saveBasedName = m_saveBasedName + "." + szSuffix;
}

void QVideoInfo::Start()
{
	QString OutputFile = m_savePath + "/" + m_saveBasedName;

	if (!m_transcoder.OpenInputFile(m_filename.toStdString()))
		return;

	m_transcoder.SetOutputFormat(m_outWith, m_outHeight);

	m_transcoder.OpenOutputFile(OutputFile.toStdString(), true, true);

	m_transcoder.Start(this);
}

void QVideoInfo::SetProgress(int nValue)
{
	QRect rc = rect();
	rc.setWidth((nValue * 1.0f) / 100.0f * rect().width());
	m_AreaRect = rc;

	QString szStatus = QString("%1%").arg(nValue);
	if (nValue == 100)
	{
		szStatus = "已完成";
		m_AreaRect = QRect();
	}
	ui->label_status->setText(szStatus);

	update();
}

void QVideoInfo::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter p(this);
	p.setPen(Qt::NoPen);
	p.setBrush(Qt::green);
	p.drawRect(m_AreaRect);

}

void QVideoInfo::ProgressValue(int second_time)
{
	static int curr_second = 0;
	static int prev_second = 0;

	float percentage = (second_time * 1.0f) / m_duration;
	curr_second = percentage * 100;
	if (curr_second != prev_second)
	{
		prev_second = curr_second;
		QRect rc = rect();
		rc.setWidth(percentage * rect().width());
		m_AreaRect = rc;

		QString szStatus = QString("%1%").arg(curr_second);
		if (percentage == 1)
		{
			curr_second = 0;
			prev_second = 0;
			szStatus = "已完成";
			m_AreaRect = QRect();
		}
		ui->label_status->setText(szStatus);

		update();
	}	
}

bool QVideoInfo::Open(const QString& szFile)
{
	std::string szText = szFile.toStdString();
	const char* pszFilename = szText.c_str();
	AVFormatContext* pFormatCtx = nullptr;
	if (0 != avformat_open_input(&pFormatCtx, pszFilename, nullptr, nullptr))
		return false;

	if (0 > avformat_find_stream_info(pFormatCtx, nullptr))
		return false;

	float duration = (pFormatCtx->duration*1.0) / (AV_TIME_BASE*1.0);
	int vidx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	int aidx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	int w = pFormatCtx->streams[vidx]->codecpar->width;
	int height = pFormatCtx->streams[vidx]->codecpar->height;
	avformat_free_context(pFormatCtx);

	QFileInfo fileInfo(szFile);
	QString filename = fileInfo.baseName(); //fileInfo.fileName();
	QString format = fileInfo.suffix(); 
	m_saveBasedName = filename; // 保存文件basename
	int filesize = fileInfo.size();
		
	int h = duration / 3600;
	int m = (duration - (3600 * h)) / 60;
	int s = int(duration - (3600 * h)) % 60;

	ui->labelFileName->setText(filename+"."+format);
	ui->labelFormat->setText(QString("%1: %2").arg("格式").arg(format.toUpper()));
	ui->labelDuration->setText(QString("%1: %2:%3:%4").arg("时长").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')));
	ui->labelSize->setText(QString("%1: %2 MB").arg("大小").arg(filesize * 1.00 / 1024 / 1024));
	ui->labelRevolutionRatio->setText(QString("%1: %2x%3").arg("分辨率").arg(w).arg(height));
	m_duration = duration;
	return true;
}