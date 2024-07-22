#include "VideoCutupWnd.h"
#include "ui_VideoCutup.h"

QVideoCutupWnd::QVideoCutupWnd(QWidget *parent) : QWidget(parent)
{
	ui = new Ui::QVideoCutupWnd();
	ui->setupUi(this);
}

QVideoCutupWnd::~QVideoCutupWnd()
{
	delete ui;
}
