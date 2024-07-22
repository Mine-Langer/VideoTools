#pragma once
#include "Common.h"
#include <QWidget>
#include <QVector>
#include <QPair>
#include "ui_FormatConversion.h"


class QFormatConversion : public QWidget//, public IDemuxEvent, public IDecoderEvent, public IRemuxEvent
{
	Q_OBJECT

public:
	QFormatConversion(QWidget *parent = Q_NULLPTR, AVType avType = TVideo);
	~QFormatConversion();


signals:
	void CvtStatusSig(int ntype);

private:
	void InitView();
	void AddFileToList(const QString& szFile);

	void Release();

private:
	void ConvertThread();

private slots:
	void OnBtnAddFileClick();
	void OnBtnAddPathClicked();
	void OnBtnStartClick();
	void CvtStatusSlot(int ntype);
	void OnBtnChangeDirClicked();
	void OnBtnOpenPathClicked();
	void OnBtnOutputFmtClicked();

private:
	Ui::QFormatConversion ui;

	QVector<QPair<QString, class QVideoInfo*>> m_vecFiles;

// 	CDemultiplexer	m_demux;	// 解封装视频文件
// 	CRemultiplexer	m_remux;	// 重封装视频文件
// 	CVideoDecoder	m_videoDecoder;
// 	CAudioDecoder	m_audioDecoder;

	AVType m_avType = TVideo;

	QString m_szOutName = "output.flv";		// 转换后文件名
	int m_outputBitRate = 320000;
	int m_nOutWidth = 1280;
	int m_nOutHeight = 720;


	QString m_szOutputPath;		// 输出路径
	QString m_szOutputSuffix;	// 输出格式
	QString m_szOutputParam;	// 输出参数

	std::thread m_convertThread;
};