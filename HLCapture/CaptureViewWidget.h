#pragma once

// #include <QWidget>
// #define PADDING 2
// enum Direction { UP = 0, DOWN = 1, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE };
// 
// class CaptureViewWidget : public QWidget
// {
// 	Q_OBJECT
// 
// public:
// 	CaptureViewWidget(QWidget *parent = nullptr);
// 	~CaptureViewWidget();
// 
// private:
// 	void region(const QPoint& cursorGlobalPoint);
// 
// protected:
// 	void mouseReleaseEvent(QMouseEvent* event) override;
// 	void mousePressEvent(QMouseEvent* e) override;
// 	void mouseMoveEvent(QMouseEvent* e) override;
// 	void paintEvent(QPaintEvent* event) override;
// 
// private:
// 	int boundaryWidth;
// 	QPoint clickPos;
// 	bool isLeftPressDown = false;  // 判断左键是否按下
// 	QPoint dragPosition;   // 窗口移动拖动时需要记住的点 
// 	Direction dir;        // 窗口大小改变时，记录改变方向
// 	QPixmap m_Pixmap;
// };

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScreen>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QPainter>


//#include "my_log.h"

enum ResizeType
{
	None,
	Left,
	Right,
	Top,
	Bottom,
	LeftTop,
	RightTop,
	LeftBottom,
	RightBottom
};

class CapturingDialog : public QDialog
{
	Q_OBJECT
public:
	explicit CapturingDialog(QWidget* parent = nullptr);
	~CapturingDialog();

private:
	void SetupUI(QDialog* dialog);
	void RetranslateUi(QDialog* dialog);

private:

	//resize and move param
	QDesktopWidget* desktop_p;
	bool resize_;
	bool move_;
	bool left_button_down_;
	ResizeType resize_type_;
	QPoint cursor_relative_window_pos_at_btn_down_;
	QPoint cursor_global_pos_at_btn_down_;
	QRect left_mouse_down_window_rect_;

	//resize mark
	int mark_wide_;
	int mark_length_;
	int mark_margin_board_;
	QColor mark_color_;

	//resize
	int resize_detect_width_;
	int resize_border_margin_;
	int resize_border_line_wide_;
	int capture_border_margin_;
	QColor resize_boder_color_;
protected:

	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);
	void leaveEvent(QEvent*);

private:
	void LimitMovePos(QPoint pt);
	void HandleResize();
	ResizeType GetResizeType(QPoint clientPos);
	void DrawResizeMark(QPainter& painter);
	void InitResizeParam();


private slots:
	//函数名规则：on_子对象名_信号名
	void on_captureButton_clicked();

private:
	QPushButton* capture_button_;
};
