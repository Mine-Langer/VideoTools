#include "AVTextureBar.h"
#include <QMouseEvent>
#include <QPainter>

AVTextureBar::AVTextureBar(QWidget *parent)
	: QWidget(parent)
{
	setMouseTracking(true);
}

AVTextureBar::~AVTextureBar()
{
}

void AVTextureBar::SetTimeLength(int duration)
{
	m_duration = duration;
}

void AVTextureBar::AddTexture(QString szFile, int type)
{
	int tSecPoint = m_varList.size() == 0 ? 0 : m_tempPos.x();
	
	QPixmap pixmap;
	if (type == 2)
		pixmap = QPixmap(":/HLCapture/res/music.png");
	else if (type == 1)
		pixmap = QPixmap(szFile);

	float ratio = (pixmap.width() * 1.0) / (pixmap.height() * 1.0);
	int imgHeight = this->height() - 2;
	int imgWidth = ratio * imgHeight;
	pixmap = pixmap.scaled(imgWidth, imgHeight);
	int length = this->width() - tSecPoint;

	ItemElem elem;
	elem.filename = szFile;
	elem.imgRect = QRectF(tSecPoint, 1, imgWidth, imgHeight);
	elem.pixmap = pixmap;
	elem.length = length;
	elem.type = m_type;

	m_varList.push_back(elem);

	UpdateLoction();
}

void AVTextureBar::UpdatePostion()
{
	m_selectedPos = m_tempPos;
}

std::vector<ItemElem>& AVTextureBar::GetItemList()
{
	return m_varList;
}

void AVTextureBar::mousePressEvent(QMouseEvent* event)
{
	if (m_bHover)	
		m_bMoved = true;
}

void AVTextureBar::mouseReleaseEvent(QMouseEvent* event)
{
	m_nSel = -1;
	if (m_bMoved) {
		setCursor(Qt::ArrowCursor);
		m_bMoved = false;
	}
}

void AVTextureBar::mouseMoveEvent(QMouseEvent* event)
{
	int PosX = event->pos().x() + 10;
	if (event->pos().x() + 40 > width())
		PosX = event->pos().x() - 30;
	m_timePos = { PosX, height() / 2 };
	m_tempPos = event->pos();
	int timePos = (event->pos().x() * 1.00) / (width() * 1.00) * m_duration;
	m_szTime = QString("%1:%2").arg(timePos / 60, 2, 10, QLatin1Char('0')).arg(timePos % 60, 2, 10, QLatin1Char('0'));

	m_bHover = false;
	for (int i = 1; i < m_varList.size(); i++)
	{
		if (m_varList[i].imgRect.left() - 2 < event->pos().x() && m_varList[i].imgRect.left() + 2 > event->pos().x())
		{
			m_nSel = i;
			m_bHover = true;
			break;
		}
	}

	if (m_bHover)
		setCursor(Qt::SizeHorCursor);
	else
		setCursor(Qt::ArrowCursor);

	if (m_bMoved)
	{
		int nw = m_varList[m_nSel].imgRect.width();
		m_varList[m_nSel].imgRect.setLeft(event->pos().x());
		m_varList[m_nSel].imgRect.setWidth(nw);
		UpdateLoction();
	}
	
	update();
}

void AVTextureBar::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	
	// 绘制游标
	for (auto it = m_varList.begin(); it != m_varList.end(); it++)
	{
		painter.drawPixmap(it->imgRect.topLeft(), it->pixmap);
		painter.setPen(Qt::green);
		painter.drawLine(it->imgRect.right(), it->imgRect.height()/2, it->length+ it->imgRect.right(), it->imgRect.height() / 2);
	}


	// 绘制时间刻度
	if (m_bDraw)
	{
		QFont font("微软雅黑", 8);
		painter.setFont(font);
		painter.setPen(Qt::white);
		painter.drawText(m_timePos, m_szTime);
	}
}

void AVTextureBar::enterEvent(QEvent* event)
{
	m_bDraw = true;
	update();
}

void AVTextureBar::leaveEvent(QEvent* event)
{
	m_bDraw = false;
	update();
}

void AVTextureBar::UpdateLoction()
{
	int i = 0;
	std::sort(m_varList.begin(), m_varList.end(), [&](const ItemElem& elemA, const ItemElem& elemB) { return elemA.imgRect.left() < elemB.imgRect.left(); });
	for (i = 0; i < m_varList.size()-1; i++)
	{
		m_varList[i].length = m_varList[i + 1].imgRect.left();
	}
	m_varList[i].length = this->width() - m_varList[i].imgRect.left();
}
