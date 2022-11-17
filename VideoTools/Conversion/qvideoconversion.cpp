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

bool QVideoConversion::ParseFile(const QString& szFile)
{
	return false;
}


void QVideoConversion::OnBtnAddFileClick()
{
	QString szFileName = QFileDialog::getOpenFileName(this, QStringLiteral("请选择视频文件"), "", "All Format (*.swf *.webm *.avi *.mp4 *.mov *.flv *.mpeg *.rm *.rmvb *.asf *.f4v *.mkv *.wmv)");
	if (szFileName.isEmpty())
		return;

	ParseFile(szFileName);

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
		m_demux.Open(cfilename);
	}
	
}