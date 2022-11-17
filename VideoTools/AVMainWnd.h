#pragma once

#include <QWidget>
#include "ui_AVMainWnd.h"
#include "capture/hlcapture.h"
#include "Composition/CompositeView.h"
#include "Conversion/qvideoconversion.h"

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
	HLCapture m_capture; //录屏
	CompositeView m_compositeView; // 合成
	QVideoConversion m_conversion; // 格式转换
	QWidget* m_pCurrentWidget = nullptr;
};
