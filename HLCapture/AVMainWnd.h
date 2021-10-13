#pragma once

#include <QWidget>
#include "ui_AVMainWnd.h"

class AVMainWnd : public QWidget
{
	Q_OBJECT

public:
	AVMainWnd(QWidget *parent = Q_NULLPTR);
	~AVMainWnd();

private:
	Ui::AVMainWnd ui;
};
