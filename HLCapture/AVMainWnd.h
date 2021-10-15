#pragma once

#include <QWidget>
#include "ui_AVMainWnd.h"
#include "hlcapture.h"

class AVMainWnd : public QWidget
{
	Q_OBJECT

public:
	AVMainWnd(QWidget *parent = Q_NULLPTR);
	~AVMainWnd();


private slots:
	void OnBtnMenuClicked();

private:
	Ui::AVMainWnd ui;
	HLCapture m_capture;
	QWidget* m_pCurrentWidget = nullptr;
};
