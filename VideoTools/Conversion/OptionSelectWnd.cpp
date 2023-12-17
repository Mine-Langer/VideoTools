#include "OptionSelectWnd.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

OptionSelectWnd::OptionSelectWnd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

	Init();

	connect(ui.listFormat, &QListWidget::currentItemChanged, this, &OptionSelectWnd::listFormatItemChanged);
	connect(ui.listQuality, &QListWidget::itemPressed, this, &OptionSelectWnd::listQualityItemPressed);
}

OptionSelectWnd::~OptionSelectWnd()
{

}

void OptionSelectWnd::setType(AVType type)
{
	avType = type;
	if (avType == TAudio) {
		ui.label_title->setText("“Ù∆µ");
		QJsonArray jsArr = m_fmtConfig.value("audio").toArray();
		for (int i = 0; i < jsArr.size(); i++) {
			ui.listFormat->addItem(jsArr.at(i).toObject().value("type").toString());
		}
		
	}
	else if (avType == TVideo) {
		ui.label_title->setText(" ”∆µ");
		QJsonArray jsArr = m_fmtConfig.value("video").toArray();
		for (int i = 0; i < jsArr.size(); i++) {
			ui.listFormat->addItem(jsArr.at(i).toObject().value("type").toString());
		}
	}
}

void OptionSelectWnd::Init()
{
	QFile file(":/AvTools/format.json");
	if (!file.open(QIODevice::ReadOnly))
		return;
	QByteArray data(file.readAll());
	file.close();

	QJsonDocument jsDoc = QJsonDocument::fromJson(data);
	m_fmtConfig = jsDoc.object();
}

void OptionSelectWnd::listFormatItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	QString szItemVal = current->text();
	int idx = ui.listFormat->row(current);
	QString fmt;
	if (avType == TAudio)
		fmt = "audio";
	else if (avType == TVideo)
		fmt = "video";

	QJsonArray jsArr = m_fmtConfig.value(fmt).toArray();
	QVariantList varParamList = jsArr.at(idx).toObject().value("param").toArray().toVariantList(); 	
	QVariant varSizeList = jsArr.at(idx).toObject().value("size").toVariant();
	ui.listQuality->clear();
	for (QVariant var : varParamList)
		ui.listQuality->addItem(var.toString());
}

void OptionSelectWnd::listQualityItemPressed(QListWidgetItem* item)
{
	QString szValue = item->text();
	QString szFormat = ui.listFormat->currentItem()->text();

	emit OptSelected(szFormat, szValue);
	close();
}
