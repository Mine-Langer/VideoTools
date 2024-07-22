#pragma once
#pragma execution_character_set("utf-8")

#include <QVector>
#include <QVariantList>
#include <QVariantMap>

class AVTools 
{
public:
	static bool InitConfig();

	static QStringList AudioFormatList();

	static QStringList VideoFormatList();

	static QVariantMap ParamData(const QString& szKey);

private:
	static QVector<QString> VecAudioFormat;
	static QVector<QString> VecVideoFormat;
	static QVariantList		VariantConfigData;
};

