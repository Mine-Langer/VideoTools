#pragma once

#include <QWidget>
#include <QVector>
#include "ui_qvideoconversion.h"
#include "../Demultiplexer.h"
#include "../Remultiplexer.h"

class QVideoConversion : public QWidget, public IDemuxEvent, public IDecoderEvent, public IRemuxEvent
{
	Q_OBJECT

public:
	QVideoConversion(QWidget *parent = Q_NULLPTR);
	~QVideoConversion();


private:
	
	virtual bool DemuxPacket(AVPacket* pkt, int type) override;

	virtual bool VideoEvent(AVFrame* frame) override;

	virtual bool AudioEvent(AVFrame* frame) override;

	virtual void RemuxEvent(int nType) override;

signals:
	void CvtStatusSig(int ntype);

private slots:
	void OnBtnAddFileClick();
	void OnBtnStartClick();
	void CvtStatusSlot(int ntype);

private:
	Ui::QVideoConversion ui;

	QVector<QString> m_vecFiles;

	CDemultiplexer	m_demux;	// 解封装视频文件
	CRemultiplexer	m_remux;	// 重封装视频文件
	CVideoDecoder	m_videoDecoder;
	CAudioDecoder	m_audioDecoder;

	QString m_szOutName = "output.flv";		// 转换后文件名
	int m_nOutWidth = 1280;
	int m_nOutHeight = 720;
};
