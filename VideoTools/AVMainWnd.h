#pragma once

#include <QWidget>
#include "ui_AVMainWnd.h"

class HLCapture;
class CompositeView;
class QVideoConversion;
class QAudioConvertWnd;
class AVMainWnd : public QWidget
{
	Q_OBJECT

public:
	AVMainWnd(QWidget *parent = Q_NULLPTR);
	~AVMainWnd();


private slots:
	void OnFunctionsItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:
	Ui::AVMainWnd ui;
	HLCapture* m_capture; //录屏
	CompositeView* m_compositeView; // 合成
	QVideoConversion* m_conversion; // 格式转换
	QAudioConvertWnd* m_audioConvert;
	QWidget* m_pCurrentWidget = nullptr;
};
