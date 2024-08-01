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
	class QCaptureWidget* m_capture; //¼��
	class CompositeView* m_compositeView; // �ϳ�
	class QFormatConversion* m_conversion; // ��Ƶ��ʽת��
	class QFormatConversion* m_audioConvert;// ��Ƶ��ʽת��
	class QFormatConversion* m_videoCutup;  // ��Ƶ�ָ�
	class QFormatConversion* m_videoWatermark; // ��Ƶˮӡ
	class QFormatConversion* m_videoMerge;	// ��Ƶ�ϲ�
	class QFormatConversion* m_videoToGIF; // ��ƵתGIF

	// ��Ƶ����

	QWidget* m_pCurrentWidget = nullptr;
};
