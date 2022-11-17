#pragma once

#include <QWidget>
#include <QVector>
#include "ui_qvideoconversion.h"
#include "../Demultiplexer.h"

class QVideoConversion : public QWidget
{
	Q_OBJECT

public:
	QVideoConversion(QWidget *parent = Q_NULLPTR);
	~QVideoConversion();


private:
	bool ParseFile(const QString& szFile);
		 

private slots:
	void OnBtnAddFileClick();
	void OnBtnStartClick();

private:
	Ui::QVideoConversion ui;

	QVector<QString> m_vecFiles;

	CDemultiplexer m_demux;
};
