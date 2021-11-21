#pragma once

#include <QWidget>
#include <QList>
#include <QPair>

#pragma execution_character_set("utf-8")

class AVTextureBar : public QWidget
{
	Q_OBJECT

public:
	AVTextureBar(QWidget *parent);
	~AVTextureBar();

	void SetTimeLength(int duration);
	
	// ������ʾ����
	void SetType(int nType);

	void AddTexture(QString szFile);

	void UpdatePostion();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	bool m_bDraw = false;
	bool m_bMoved = false;
	QString m_szTime;
	QPoint m_timePos; // ����ʱ������
	QPoint m_selectedPos, m_tempPos;
	int m_duration = 0;
	int m_type = 0; // ���� 0:ͼ��  1:��Ƶ
	int m_nSel = -1;
	QList<QPair<int,QString>> m_varList;

};
