#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QList>
#include <QPair>

struct ItemElem
{
	QString	filename;
	QPixmap pixmap;
	int		type;
	QRectF	imgRect;
	int		length;
};

class AVTextureBar : public QWidget
{
	Q_OBJECT

public:
	AVTextureBar(QWidget *parent);
	~AVTextureBar();

	void SetTimeLength(int duration);
	
	void AddTexture(QString szFile, int type);

	void UpdatePostion();

	std::vector<ItemElem>& GetItemList();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	void UpdateLoction();

private:
	bool m_bDraw = false;
	bool m_bMoved = false, m_bHover = false;
	QString m_szTime;
	QPoint m_timePos; // 绘制时间坐标
	QPoint m_selectedPos, m_tempPos;
	int m_duration = 0;
	int m_type = 0; // 类型 0:图像  1:音频
	int m_nSel = -1;
	std::vector<ItemElem> m_varList;

};
