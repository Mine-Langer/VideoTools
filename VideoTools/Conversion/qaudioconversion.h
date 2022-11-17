#pragma once

#include <QWidget>
#include "ui_qaudioconversion.h"

class QAudioConversion : public QWidget
{
	Q_OBJECT

public:
	QAudioConversion(QWidget *parent = Q_NULLPTR);
	~QAudioConversion();

private:
	Ui::QAudioConversion ui;
};
