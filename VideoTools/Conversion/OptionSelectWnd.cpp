#include "OptionSelectWnd.h"

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
		ui.label_title->setText("��Ƶ");
		for (auto v = audioHashValue.begin(); v != audioHashValue.end(); v++) {
			ui.listFormat->addItem(v.key());
		}
		
	}
	else if (avType == TVideo) {
		ui.label_title->setText("��Ƶ");
	}
}

void OptionSelectWnd::Init()
{
	audioHashValue["MP3"] = { "��Ʒ��","��Ʒ��","��Ʒ��" };
	audioHashValue["WAV"] = { "����" };
	audioHashValue["M4A"] = { "��Ʒ��","��Ʒ��","��Ʒ��" };
	audioHashValue["WMA"] = { "��Ʒ��","��Ʒ��","��Ʒ��" };
	audioHashValue["AAC"] = { "��Ʒ��","��Ʒ��","��Ʒ��" };
	audioHashValue["FLAC"] = { "����" };

	audioHashValue["M4R"] = { "��Ʒ��","��Ʒ��","��Ʒ��" };
	audioHashValue["OGG"] = { "��Ʒ��","��Ʒ��","��Ʒ��" };
}

void OptionSelectWnd::listFormatItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	QString szItemVal = current->text();
	ui.listQuality->clear();
	ui.listQuality->addItems(audioHashValue[szItemVal].toList());
}

void OptionSelectWnd::listQualityItemPressed(QListWidgetItem* item)
{
	QString szValue = item->text();
	QString szFormat = ui.listFormat->currentItem()->text();

	emit OptSelected(szFormat, szValue);
	close();
}
