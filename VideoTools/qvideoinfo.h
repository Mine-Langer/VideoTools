#pragma once
#include <QWidget>
#include "Transcoder.h"

namespace Ui { class QVideoInfo; };

class QVideoInfo : public QWidget, public ITranscodeProgress
{
	Q_OBJECT

public:
	QVideoInfo(const QString& szFile, QWidget *parent = Q_NULLPTR);
	~QVideoInfo();

	// 设置保存路径
	void SetSavePath(const QString& szPath);

	void SetParam(const QString& szSuffix, const QString& szParam);

	void Start();

	// 设置进度条
	void SetProgress(int nValue);

protected:
	virtual void paintEvent(QPaintEvent* event) override;

	virtual void ProgressValue(int second_time) override;

private:
	bool Open(const QString& szFile);

private:
	Ui::QVideoInfo* ui;

	QRect	m_AreaRect;
	QString m_filename; // 输入文件名
	QString m_savePath; // 保存文件路径
	QString m_saveBasedName; // 保存文件名称

	float m_duration;	// 时长(秒)

	CTranscoder m_transcoder;
	int m_outWith = -1;
	int m_outHeight = -1;
};