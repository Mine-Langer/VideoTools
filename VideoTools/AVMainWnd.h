#pragma once

#include <QWidget>
#include "ui_AVMainWnd.h"

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
	class QCaptureWidget* m_capture; //录屏
	class CompositeView* m_compositeView; // 合成
	class QFormatConversion* m_conversion; // 视频格式转换
	class QFormatConversion* m_audioConvert;// 音频格式转换
	class QFormatConversion* m_videoCutup;  // 视频分割
	class QFormatConversion* m_videoWatermark; // 视频水印
	class QFormatConversion* m_videoMerge;	// 视频合并
	class QFormatConversion* m_videoToGIF; // 视频转GIF

	// 视频推流

	QWidget* m_pCurrentWidget = nullptr;
};
