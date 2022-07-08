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

void AVTextureBar::AddTexture(QString szFile)
{
	int tSecPoint = m_tempPos.x();
	if (m_varList.size() == 0)
	{
		tSecPoint = 0;
	}

	m_varList.append(qMakePair(tSecPoint, szFile));
}

void AVTextureBar::UpdatePostion()
{
	m_selectedPos = m_tempPos;
}

void AVTextureBar::mousePressEvent(QMouseEvent* event)
{
	for (int i = 0; i < m_varList.size(); i++)
	{
		if (m_varList[i].first > event->pos().x()-1 && m_varList[i].first < event->pos().x() + 1)
		{
			m_bMoved = true;
			m_nSel = i;
			break;
		}
	}
}

void AVTextureBar::mouseReleaseEvent(QMouseEvent* event)
{
	m_nSel = -1;
	m_bMoved = false;
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

	bool bHover = false;
	if (m_nSel != -1)
		m_varList[m_nSel].first = event->pos().x()<0?0: event->pos().x();
	else
	{
		for (int i=0; i< m_varList.size(); i++)
		{
			if (m_varList[i].first > event->pos().x() - 1 && m_varList[i].first < event->pos().x() + 1)
			{
				bHover = true;				
				break;
			}
		}
	}
	if (bHover)
		setCursor(Qt::SizeHorCursor);
	else
		setCursor(Qt::ArrowCursor);

	update();
}

void AVTextureBar::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	// 绘制时间刻度
	if (m_bDraw)
	{
		QFont font("微软雅黑", 8);
		painter.setFont(font);
		painter.setPen(Qt::white);
		painter.drawText(m_timePos, m_szTime);
	}

	// 绘制游标
	for (auto it = m_varList.begin(); it != m_varList.end(); it++)
	{
		int posX = it->first;
		if (posX < 0)
			posX = 0;
		painter.setPen(Qt::green);
		painter.drawLine(posX, 0, posX, height());
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
