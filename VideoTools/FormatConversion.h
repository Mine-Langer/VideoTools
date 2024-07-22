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

// 	CDemultiplexer	m_demux;	// ���װ��Ƶ�ļ�
// 	CRemultiplexer	m_remux;	// �ط�װ��Ƶ�ļ�
// 	CVideoDecoder	m_videoDecoder;
// 	CAudioDecoder	m_audioDecoder;

	AVType m_avType = TVideo;

	QString m_szOutName = "output.flv";		// ת�����ļ���
	int m_outputBitRate = 320000;
	int m_nOutWidth = 1280;
	int m_nOutHeight = 720;


	QString m_szOutputPath;		// ���·��
	QString m_szOutputSuffix;	// �����ʽ
	QString m_szOutputParam;	// �������

	std::thread m_convertThread;
};