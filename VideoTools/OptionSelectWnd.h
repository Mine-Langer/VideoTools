#pragma once
#include "Common.h"
#include <QWidget>
#include <QJsonObject>
#include <QHash>
#include <QVector>
#include "ui_optionSelect.h"

class OptionSelectWnd  : public QWidget
{
	Q_OBJECT

public:
	OptionSelectWnd(QWidget *parent = nullptr);
	~OptionSelectWnd();

	void setType(AVType type);

signals:
	void OptSelected(QString, QString, QString);

private:
//	void Init();


private slots:
	void listFormatItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void listQualityItemPressed(QListWidgetItem* item);

private:
	Ui::OptionSelectWnd ui;

	AVType avType;
//	QJsonObject m_fmtConfig;
};