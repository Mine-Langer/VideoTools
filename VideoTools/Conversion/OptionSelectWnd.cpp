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
		ui.label_title->setText("音频");
		for (auto v = audioHashValue.begin(); v != audioHashValue.end(); v++) {
			ui.listFormat->addItem(v.key());
		}
		
	}
	else if (avType == TVideo) {
		ui.label_title->setText("视频");
	}
}

void OptionSelectWnd::Init()
{
	audioHashValue["MP3"] = { "高品质","中品质","低品质" };
	audioHashValue["WAV"] = { "无损" };
	audioHashValue["M4A"] = { "高品质","中品质","低品质" };
	audioHashValue["WMA"] = { "高品质","中品质","低品质" };
	audioHashValue["AAC"] = { "高品质","中品质","低品质" };
	audioHashValue["FLAC"] = { "无损" };

	audioHashValue["M4R"] = { "高品质","中品质","低品质" };
	audioHashValue["OGG"] = { "高品质","中品质","低品质" };
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
