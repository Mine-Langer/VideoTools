#pragma once

#include <QWidget>
#include <QVector>
#include "ui_qvideoconversion.h"
#include "../Demultiplexer.h"
#include "../Remultiplexer.h"

class QVideoConversion : public QWidget, public IDemuxEvent, public IDecoderEvent
{
	Q_OBJECT

public:
	QVideoConversion(QWidget *parent = Q_NULLPTR);
	~QVideoConversion();


private:
	
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;

	virtual bool VideoEvent(AVFrame* frame) override;

	virtual bool AudioEvent(AVFrame* frame) override;

private slots:
	void OnBtnAddFileClick();
	void OnBtnStartClick();

private:
	Ui::QVideoConversion ui;

	QVector<QString> m_vecFiles;

	CDemultiplexer	m_demux;	// ���װ��Ƶ�ļ�
	CRemultiplexer	m_remux;	// �ط�װ��Ƶ�ļ�
	CVideoDecoder	m_videoDecoder;
	CAudioDecoder	m_audioDecoder;

	QString m_szOutName = "output.mkv";		// ת�����ļ���
	int m_nOutWidth = 1280;
	int m_nOutHeight = 720;
};
