#pragma once

#include <QWidget>
#include <QMenu>
#include "Composite.h"
#include "AVTextureBar.h"
#include "../player.h"

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
	void OnActImageDel();
	void OnActAudioDel();
	void OnBtnPlay();
	void OnBtnExport();
	void timeDurationChanged(const QTime& time);
	void sliderTimeMoved(int position);

private:
	Ui::CompositeView *ui;
	QMenu* m_imageMenu = nullptr;
	QMenu* m_audioMenu = nullptr;

	Composite m_composite;
	CPlayer		m_player;
	int m_comType = 0;

	QList<ItemElem> m_itemImageList;
	QList<ItemElem> m_itemMusicList;
};
