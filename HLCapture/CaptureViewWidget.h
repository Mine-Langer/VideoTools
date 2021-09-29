#pragma once

#include <QWidget>
#define PADDING 2
enum Direction { UP = 0, DOWN = 1, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE };

class CaptureViewWidget : public QWidget
{
	Q_OBJECT

public:
	CaptureViewWidget(QWidget *parent = nullptr);
	~CaptureViewWidget();

private:
	void region(const QPoint& cursorGlobalPoint);

protected:
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

private:
	int boundaryWidth;
	QPoint clickPos;
	bool isLeftPressDown;  // �ж�����Ƿ���
	QPoint dragPosition;   // �����ƶ��϶�ʱ��Ҫ��ס�ĵ� 
	Direction dir;        // ���ڴ�С�ı�ʱ����¼�ı䷽��
};
