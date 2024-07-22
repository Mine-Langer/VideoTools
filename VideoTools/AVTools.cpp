#include "AVTools.h"
#include <QFile>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QVector<QString> AVTools::VecAudioFormat;
QVector<QString> AVTools::VecVideoFormat;
QVariantList	 AVTools::VariantConfigData;

bool AVTools::InitConfig()
{
	QFile file(":/AvTools/res/format.json");
	if (!file.open(QIODevice::ReadOnly))
		return false;

	QByteArray data(file.readAll());
	file.close();

	QString szKey;
	QJsonDocument jsDoc = QJsonDocument::fromJson(data);
	QJsonObject fmtConfig = jsDoc.object();
	QJsonArray jsArr = fmtConfig.value("audio").toArray();
	for (int i = 0; i < jsArr.size(); i++) 
	{
		VecAudioFormat.append(jsArr.at(i).toObject().value("type").toString());

		VariantConfigData.append(jsArr.at(i).toObject().toVariantMap());
	}

	jsArr = fmtConfig.value("video").toArray();
	for (int i = 0; i < jsArr.size(); i++) 
	{
		VecVideoFormat.append(jsArr.at(i).toObject().value("type").toString());

		VariantConfigData.append(jsArr.at(i).toObject().toVariantMap());
	}

	return true;
}

QStringList AVTools::AudioFormatList()
{
	return QStringList::fromVector(VecAudioFormat);
}

QStringList AVTools::VideoFormatList()
{
	return QStringList::fromVector(VecVideoFormat);
}

QVariantMap AVTools::ParamData(const QString& szKey)
{
	for (int i=0; i<VariantConfigData.size(); i++)
	{
		QVariantMap var = VariantConfigData.at(i).toMap();
		if (var.value("type").toString() == szKey)
		{
			return var;
		}
	}
	return QVariantMap();
}
