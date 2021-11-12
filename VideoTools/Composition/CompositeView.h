#pragma once

#include <QWidget>
#include <QMenu>
#include "Composite.h"
#pragma execution_character_set("utf-8")

namespace Ui { class CompositeView; };

class CompositeView : public QWidget
{
	Q_OBJECT

public:
	CompositeView(QWidget *parent = Q_NULLPTR);
	~CompositeView();


protected:
	void showEvent(QShowEvent* event) override;

private slots:
	void OnImageWidgetContextMenuRequested(const QPoint& pos);
	void OnAudioWidgetContextMenuRequested(const QPoint& pos);
	void OnActImage();
	void OnActAudio();
	void OnBtnPlay();
	void OnBtnExport();

private:
	Ui::CompositeView *ui;
	QMenu* m_imageMenu = nullptr;
	QMenu* m_audioMenu = nullptr;

	Composite m_composite;
};
