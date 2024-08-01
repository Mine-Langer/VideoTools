/********************************************************************************
** Form generated from reading UI file 'CaptureView.ui'
**
** Created by: Qt User Interface Compiler version 5.12.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAPTUREVIEW_H
#define UI_CAPTUREVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CaptureWidget
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QRadioButton *radioScreenCap;
    QSpacerItem *verticalSpacer;
    QRadioButton *radioAreaCap;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QRadioButton *radioAudioAll;
    QRadioButton *radioAudioSys;
    QRadioButton *radioAudioMic;
    QRadioButton *radioAudioNoCap;
    QSpacerItem *verticalSpacer_2;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_3;
    QRadioButton *radioStandardQuality;
    QRadioButton *radioHighQuality;
    QRadioButton *radiOriginalQuality;
    QSpacerItem *verticalSpacer_3;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_4;
    QRadioButton *radioMP4;
    QRadioButton *radioFLV;
    QRadioButton *radioAVI;
    QRadioButton *radioMP3;
    QRadioButton *radioEXE;
    QSpacerItem *verticalSpacer_4;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *checkScreenPaint;
    QCheckBox *checkSuspension;
    QCheckBox *checkTimingStop;
    QSpacerItem *horizontalSpacer_2;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLabel *labelCapDuration;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnStartCap;
    QWidget *widget_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QLineEdit *editOutputPath;
    QPushButton *btnChangeDir;
    QPushButton *btnOpenDir;
    QSpacerItem *horizontalSpacer_3;

    void setupUi(QWidget *CaptureWidget)
    {
        if (CaptureWidget->objectName().isEmpty())
            CaptureWidget->setObjectName(QString::fromUtf8("CaptureWidget"));
        CaptureWidget->resize(714, 542);
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font.setPointSize(13);
        CaptureWidget->setFont(font);
        verticalLayout = new QVBoxLayout(CaptureWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        widget = new QWidget(CaptureWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        groupBox = new QGroupBox(widget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setLayoutDirection(Qt::LeftToRight);
        groupBox->setAlignment(Qt::AlignCenter);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetNoConstraint);
        gridLayout->setVerticalSpacing(20);
        radioScreenCap = new QRadioButton(groupBox);
        radioScreenCap->setObjectName(QString::fromUtf8("radioScreenCap"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(radioScreenCap->sizePolicy().hasHeightForWidth());
        radioScreenCap->setSizePolicy(sizePolicy);

        gridLayout->addWidget(radioScreenCap, 0, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 2, 0, 1, 1);

        radioAreaCap = new QRadioButton(groupBox);
        radioAreaCap->setObjectName(QString::fromUtf8("radioAreaCap"));
        sizePolicy.setHeightForWidth(radioAreaCap->sizePolicy().hasHeightForWidth());
        radioAreaCap->setSizePolicy(sizePolicy);

        gridLayout->addWidget(radioAreaCap, 1, 0, 1, 1);


        horizontalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(widget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setAlignment(Qt::AlignCenter);
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setSizeConstraint(QLayout::SetNoConstraint);
        gridLayout_2->setVerticalSpacing(20);
        radioAudioAll = new QRadioButton(groupBox_2);
        radioAudioAll->setObjectName(QString::fromUtf8("radioAudioAll"));
        sizePolicy.setHeightForWidth(radioAudioAll->sizePolicy().hasHeightForWidth());
        radioAudioAll->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(radioAudioAll, 0, 0, 1, 1);

        radioAudioSys = new QRadioButton(groupBox_2);
        radioAudioSys->setObjectName(QString::fromUtf8("radioAudioSys"));
        sizePolicy.setHeightForWidth(radioAudioSys->sizePolicy().hasHeightForWidth());
        radioAudioSys->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(radioAudioSys, 1, 0, 1, 1);

        radioAudioMic = new QRadioButton(groupBox_2);
        radioAudioMic->setObjectName(QString::fromUtf8("radioAudioMic"));
        sizePolicy.setHeightForWidth(radioAudioMic->sizePolicy().hasHeightForWidth());
        radioAudioMic->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(radioAudioMic, 2, 0, 1, 1);

        radioAudioNoCap = new QRadioButton(groupBox_2);
        radioAudioNoCap->setObjectName(QString::fromUtf8("radioAudioNoCap"));
        sizePolicy.setHeightForWidth(radioAudioNoCap->sizePolicy().hasHeightForWidth());
        radioAudioNoCap->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(radioAudioNoCap, 3, 0, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_2, 4, 0, 1, 1);


        horizontalLayout->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(widget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setAlignment(Qt::AlignCenter);
        gridLayout_3 = new QGridLayout(groupBox_3);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setVerticalSpacing(20);
        radioStandardQuality = new QRadioButton(groupBox_3);
        radioStandardQuality->setObjectName(QString::fromUtf8("radioStandardQuality"));
        sizePolicy.setHeightForWidth(radioStandardQuality->sizePolicy().hasHeightForWidth());
        radioStandardQuality->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(radioStandardQuality, 0, 0, 1, 1);

        radioHighQuality = new QRadioButton(groupBox_3);
        radioHighQuality->setObjectName(QString::fromUtf8("radioHighQuality"));

        gridLayout_3->addWidget(radioHighQuality, 1, 0, 1, 1);

        radiOriginalQuality = new QRadioButton(groupBox_3);
        radiOriginalQuality->setObjectName(QString::fromUtf8("radiOriginalQuality"));

        gridLayout_3->addWidget(radiOriginalQuality, 2, 0, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_3->addItem(verticalSpacer_3, 3, 0, 1, 1);


        horizontalLayout->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(widget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setAlignment(Qt::AlignCenter);
        gridLayout_4 = new QGridLayout(groupBox_4);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setVerticalSpacing(20);
        gridLayout_4->setContentsMargins(-1, -1, -1, 9);
        radioMP4 = new QRadioButton(groupBox_4);
        radioMP4->setObjectName(QString::fromUtf8("radioMP4"));
        sizePolicy.setHeightForWidth(radioMP4->sizePolicy().hasHeightForWidth());
        radioMP4->setSizePolicy(sizePolicy);

        gridLayout_4->addWidget(radioMP4, 0, 0, 1, 1);

        radioFLV = new QRadioButton(groupBox_4);
        radioFLV->setObjectName(QString::fromUtf8("radioFLV"));

        gridLayout_4->addWidget(radioFLV, 1, 0, 1, 1);

        radioAVI = new QRadioButton(groupBox_4);
        radioAVI->setObjectName(QString::fromUtf8("radioAVI"));

        gridLayout_4->addWidget(radioAVI, 2, 0, 1, 1);

        radioMP3 = new QRadioButton(groupBox_4);
        radioMP3->setObjectName(QString::fromUtf8("radioMP3"));

        gridLayout_4->addWidget(radioMP3, 3, 0, 1, 1);

        radioEXE = new QRadioButton(groupBox_4);
        radioEXE->setObjectName(QString::fromUtf8("radioEXE"));

        gridLayout_4->addWidget(radioEXE, 4, 0, 1, 1);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_4->addItem(verticalSpacer_4, 5, 0, 1, 1);


        horizontalLayout->addWidget(groupBox_4);


        verticalLayout->addWidget(widget);

        widget_2 = new QWidget(CaptureWidget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy1);
        widget_2->setMinimumSize(QSize(0, 50));
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setSpacing(12);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        checkScreenPaint = new QCheckBox(widget_2);
        checkScreenPaint->setObjectName(QString::fromUtf8("checkScreenPaint"));

        horizontalLayout_2->addWidget(checkScreenPaint);

        checkSuspension = new QCheckBox(widget_2);
        checkSuspension->setObjectName(QString::fromUtf8("checkSuspension"));

        horizontalLayout_2->addWidget(checkSuspension);

        checkTimingStop = new QCheckBox(widget_2);
        checkTimingStop->setObjectName(QString::fromUtf8("checkTimingStop"));

        horizontalLayout_2->addWidget(checkTimingStop);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addWidget(widget_2);

        widget_3 = new QWidget(CaptureWidget);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        sizePolicy1.setHeightForWidth(widget_3->sizePolicy().hasHeightForWidth());
        widget_3->setSizePolicy(sizePolicy1);
        widget_3->setMinimumSize(QSize(0, 80));
        horizontalLayout_3 = new QHBoxLayout(widget_3);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, -1, 20, -1);
        label = new QLabel(widget_3);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        labelCapDuration = new QLabel(widget_3);
        labelCapDuration->setObjectName(QString::fromUtf8("labelCapDuration"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font1.setPointSize(32);
        font1.setBold(false);
        font1.setWeight(50);
        labelCapDuration->setFont(font1);
        labelCapDuration->setStyleSheet(QString::fromUtf8("color: rgb(60, 170, 82);"));

        horizontalLayout_3->addWidget(labelCapDuration);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        btnStartCap = new QPushButton(widget_3);
        btnStartCap->setObjectName(QString::fromUtf8("btnStartCap"));
        sizePolicy.setHeightForWidth(btnStartCap->sizePolicy().hasHeightForWidth());
        btnStartCap->setSizePolicy(sizePolicy);
        btnStartCap->setMinimumSize(QSize(100, 30));

        horizontalLayout_3->addWidget(btnStartCap);


        verticalLayout->addWidget(widget_3);

        widget_4 = new QWidget(CaptureWidget);
        widget_4->setObjectName(QString::fromUtf8("widget_4"));
        sizePolicy1.setHeightForWidth(widget_4->sizePolicy().hasHeightForWidth());
        widget_4->setSizePolicy(sizePolicy1);
        widget_4->setMinimumSize(QSize(0, 80));
        horizontalLayout_4 = new QHBoxLayout(widget_4);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(widget_4);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        editOutputPath = new QLineEdit(widget_4);
        editOutputPath->setObjectName(QString::fromUtf8("editOutputPath"));
        sizePolicy.setHeightForWidth(editOutputPath->sizePolicy().hasHeightForWidth());
        editOutputPath->setSizePolicy(sizePolicy);
        editOutputPath->setMinimumSize(QSize(400, 0));

        horizontalLayout_4->addWidget(editOutputPath);

        btnChangeDir = new QPushButton(widget_4);
        btnChangeDir->setObjectName(QString::fromUtf8("btnChangeDir"));

        horizontalLayout_4->addWidget(btnChangeDir);

        btnOpenDir = new QPushButton(widget_4);
        btnOpenDir->setObjectName(QString::fromUtf8("btnOpenDir"));

        horizontalLayout_4->addWidget(btnOpenDir);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);


        verticalLayout->addWidget(widget_4);


        retranslateUi(CaptureWidget);

        QMetaObject::connectSlotsByName(CaptureWidget);
    } // setupUi

    void retranslateUi(QWidget *CaptureWidget)
    {
        CaptureWidget->setWindowTitle(QApplication::translate("CaptureWidget", "\345\261\217\345\271\225\345\275\225\345\210\266", nullptr));
        groupBox->setTitle(QApplication::translate("CaptureWidget", "\350\247\206\351\242\221\351\200\211\351\241\271", nullptr));
        radioScreenCap->setText(QApplication::translate("CaptureWidget", "\345\205\250\345\261\217\345\275\225\345\210\266", nullptr));
        radioAreaCap->setText(QApplication::translate("CaptureWidget", "\345\214\272\345\237\237\345\275\225\345\210\266", nullptr));
        groupBox_2->setTitle(QApplication::translate("CaptureWidget", "\351\237\263\351\242\221\351\200\211\351\241\271", nullptr));
        radioAudioAll->setText(QApplication::translate("CaptureWidget", "\345\205\250\351\203\250\345\275\225\345\210\266", nullptr));
        radioAudioSys->setText(QApplication::translate("CaptureWidget", "\344\273\205\347\263\273\347\273\237\345\243\260\351\237\263", nullptr));
        radioAudioMic->setText(QApplication::translate("CaptureWidget", "\344\273\205\351\272\246\345\205\213\351\243\216\345\243\260\351\237\263", nullptr));
        radioAudioNoCap->setText(QApplication::translate("CaptureWidget", "\344\270\215\345\275\225\345\243\260\351\237\263", nullptr));
        groupBox_3->setTitle(QApplication::translate("CaptureWidget", "\347\224\273\350\264\250\350\256\276\347\275\256", nullptr));
        radioStandardQuality->setText(QApplication::translate("CaptureWidget", "\346\240\207\346\270\205", nullptr));
        radioHighQuality->setText(QApplication::translate("CaptureWidget", "\351\253\230\346\270\205", nullptr));
        radiOriginalQuality->setText(QApplication::translate("CaptureWidget", "\345\216\237\347\224\273", nullptr));
        groupBox_4->setTitle(QApplication::translate("CaptureWidget", "\345\275\225\345\210\266\346\240\274\345\274\217", nullptr));
        radioMP4->setText(QApplication::translate("CaptureWidget", "MP4", nullptr));
        radioFLV->setText(QApplication::translate("CaptureWidget", "FLV", nullptr));
        radioAVI->setText(QApplication::translate("CaptureWidget", "AVI", nullptr));
        radioMP3->setText(QApplication::translate("CaptureWidget", "MP3", nullptr));
        radioEXE->setText(QApplication::translate("CaptureWidget", "EXE", nullptr));
        checkScreenPaint->setText(QApplication::translate("CaptureWidget", "\345\274\200\345\220\257\345\261\217\345\271\225\347\224\273\345\233\276\345\267\245\345\205\267", nullptr));
        checkSuspension->setText(QApplication::translate("CaptureWidget", "\345\274\200\345\220\257\346\202\254\346\265\256\347\252\227", nullptr));
        checkTimingStop->setText(QApplication::translate("CaptureWidget", "\345\256\232\346\227\266\345\201\234\346\255\242\345\275\225\345\210\266", nullptr));
        label->setText(QApplication::translate("CaptureWidget", "\345\275\225\345\210\266\346\227\266\351\225\277\357\274\232", nullptr));
        labelCapDuration->setText(QApplication::translate("CaptureWidget", "00:00:00", nullptr));
        btnStartCap->setText(QApplication::translate("CaptureWidget", "\345\274\200\345\247\213", nullptr));
        label_3->setText(QApplication::translate("CaptureWidget", "\350\276\223\345\207\272\350\267\257\345\276\204\357\274\232", nullptr));
        btnChangeDir->setText(QApplication::translate("CaptureWidget", "\346\233\264\346\224\271\347\233\256\345\275\225", nullptr));
        btnOpenDir->setText(QApplication::translate("CaptureWidget", "\346\211\223\345\274\200\346\226\207\344\273\266\345\244\271", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CaptureWidget: public Ui_CaptureWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAPTUREVIEW_H
