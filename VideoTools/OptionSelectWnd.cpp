#include "OptionSelectWnd.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include "AVTools.h"

OptionSelectWnd::OptionSelectWnd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

	connect(ui.listFormat, &QListWidget::currentItemChanged, this, &OptionSelectWnd::listFormatItemChanged);
	connect(ui.listQuality, &QListWidget::itemPressed, this, &OptionSelectWnd::listQualityItemPressed);
}

OptionSelectWnd::~OptionSelectWnd()
{

}

void OptionSelectWnd::setType(AVType type)
{
	avType = type;
	ui.label_title->setText(avType == TAudio ? "“Ù∆µ" : " ”∆µ");
	QStringList FmtList = avType == TAudio ? AVTools::AudioFormatList() : AVTools::VideoFormatList();
	for (int i = 0; i < FmtList.size(); i++)
	{
		ui.listFormat->addItem(FmtList.at(i));
	}
}


void OptionSelectWnd::listFormatItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	QString szItemVal = current->text();
	int idx = 0;
	QString fmt = avType == TAudio ? "audio" : "video";

	QVariantMap VarData = AVTools::ParamData(szItemVal);
	if (VarData.isEmpty())
		return;

	QStringList varParamList = VarData.value("param").toStringList();
	QStringList varSizeList = VarData.value("size").toStringList();
	ui.listQuality->clear();
	
	for (QVariant var : varParamList)
	{
		QListWidgetItem* pItem = new QListWidgetItem(var.toString());
		if (varSizeList.size() > idx)
			pItem->setData(Qt::UserRole, varSizeList[idx++]);
		ui.listQuality->addItem(pItem);
	}
}

void OptionSelectWnd::listQualityItemPressed(QListWidgetItem* item)
{
	QString szValue = item->text();
	QString szFormat = ui.listFormat->currentItem()->text();
	QString szParam = item->data(Qt::UserRole).toString();

	emit OptSelected(szFormat, szValue, szParam);
	close();
}