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

	// ���ñ���·��
	void SetSavePath(const QString& szPath);

	void SetParam(const QString& szSuffix, const QString& szParam);

	void Start();

	// ���ý�����
	void SetProgress(int nValue);

protected:
	virtual void paintEvent(QPaintEvent* event) override;

	virtual void ProgressValue(int second_time) override;

private:
	bool Open(const QString& szFile);

private:
	Ui::QVideoInfo* ui;

	QRect	m_AreaRect;
	QString m_filename; // �����ļ���
	QString m_savePath; // �����ļ�·��
	QString m_saveBasedName; // �����ļ�����

	float m_duration;	// ʱ��(��)

	CTranscoder m_transcoder;
	int m_outWith = -1;
	int m_outHeight = -1;
};