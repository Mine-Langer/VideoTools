#pragma once

#include <QWidget>
#include "ui_AVMainWnd.h"
#include "capture/hlcapture.h"
#include "Composition/CompositeView.h"

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
	HLCapture m_capture; //Â¼ÆÁ
	CompositeView m_compositeView; // ºÏ³É
	QWidget* m_pCurrentWidget = nullptr;
};
