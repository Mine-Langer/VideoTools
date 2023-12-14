#pragma once

#include <QWidget>
#include "../Common.h"
#include "ui_optionSelect.h"
#include <QHash>
#include <QVector>

class OptionSelectWnd  : public QWidget
{
	Q_OBJECT

public:
	OptionSelectWnd(QWidget *parent = nullptr);
	~OptionSelectWnd();

	void setType(AVType type);

signals:
	void OptSelected(QString, QString);

private:
	void Init();


private slots:
	void listFormatItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void listQualityItemPressed(QListWidgetItem* item);

private:
	Ui::OptionSelectWnd ui;

	AVType avType;
	QHash<QString, QVector<QString>> videoHashValue;
	QHash<QString, QVector<QString>> audioHashValue;
};
