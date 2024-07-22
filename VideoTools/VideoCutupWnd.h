#pragma once

#include <QWidget>

namespace Ui { class QVideoCutupWnd; };

class QVideoCutupWnd  : public QWidget
{
	Q_OBJECT

public:
	QVideoCutupWnd(QWidget *parent = nullptr);
	~QVideoCutupWnd();


private:
	Ui::QVideoCutupWnd* ui;
};
