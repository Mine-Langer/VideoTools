#pragma once

#include <QWidget>
#include "ui_qvideoinfo.h"

class QVideoInfo : public QWidget
{
	Q_OBJECT

public:
	QVideoInfo(const QString& szFile, QWidget *parent = Q_NULLPTR);
	~QVideoInfo();

private:
	bool Open(const QString& szFile);

private:
	Ui::QVideoInfo ui;

	QString m_filename;
};
