#include "Common.h"
#include "qvideoinfo.h"
#include <QFileInfo>

QVideoInfo::QVideoInfo(const QString& szFile, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	Open(szFile);
}

QVideoInfo::~QVideoInfo()
{
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
	QString filename = fileInfo.fileName(); 
	QString format = fileInfo.suffix(); 
	int filesize = fileInfo.size();
		
	int h = duration / 3600;
	int m = (duration - (3600 * h)) / 60;
	int s = int(duration - (3600 * h)) % 60;

	ui.labelFileName->setText(filename);
	ui.labelFormat->setText(QString("%1: %2").arg("格式").arg(format.toUpper()));
	ui.labelDuration->setText(QString("%1: %2:%3:%4").arg("时长").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')));
	ui.labelSize->setText(QString("%1: %2 MB").arg("大小").arg(filesize * 1.00 / 1024 / 1024));
	ui.labelRevolutionRatio->setText(QString("%1: %2x%3").arg("分辨率").arg(w).arg(height));
	return true;
}
