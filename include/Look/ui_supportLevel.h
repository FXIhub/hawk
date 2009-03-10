/********************************************************************************
** Form generated from reading ui file 'supportLevel.ui'
**
** Created: Tue Mar 10 09:42:42 2009
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SUPPORTLEVEL_H
#define UI_SUPPORTLEVEL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SupportLevel
{
public:
    QVBoxLayout *vboxLayout;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QDoubleSpinBox *floorSpin;
    QSlider *floorSlider;
    QLabel *label_3;
    QDoubleSpinBox *ceilingSpin;
    QSlider *ceilingSlider;
    QLabel *label_5;
    QDoubleSpinBox *blurSpin;
    QSlider *blurSlider;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *recalculateButton;
    QSpacerItem *spacerItem1;

    void setupUi(QWidget *SupportLevel)
    {
    if (SupportLevel->objectName().isEmpty())
        SupportLevel->setObjectName(QString::fromUtf8("SupportLevel"));
    SupportLevel->resize(336, 173);
    vboxLayout = new QVBoxLayout(SupportLevel);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    gridLayout = new QGridLayout();
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label_2 = new QLabel(SupportLevel);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    QFont font;
    font.setPointSize(13);
    label_2->setFont(font);

    gridLayout->addWidget(label_2, 0, 0, 1, 1);

    floorSpin = new QDoubleSpinBox(SupportLevel);
    floorSpin->setObjectName(QString::fromUtf8("floorSpin"));
    floorSpin->setFont(font);
    floorSpin->setMinimum(75);
    floorSpin->setMaximum(100);
    floorSpin->setValue(90);

    gridLayout->addWidget(floorSpin, 0, 1, 1, 1);

    floorSlider = new QSlider(SupportLevel);
    floorSlider->setObjectName(QString::fromUtf8("floorSlider"));
    floorSlider->setMinimum(7500);
    floorSlider->setMaximum(10000);
    floorSlider->setPageStep(100);
    floorSlider->setValue(9000);
    floorSlider->setOrientation(Qt::Horizontal);
    floorSlider->setTickInterval(1);

    gridLayout->addWidget(floorSlider, 0, 2, 1, 1);

    label_3 = new QLabel(SupportLevel);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setFont(font);

    gridLayout->addWidget(label_3, 1, 0, 1, 1);

    ceilingSpin = new QDoubleSpinBox(SupportLevel);
    ceilingSpin->setObjectName(QString::fromUtf8("ceilingSpin"));
    ceilingSpin->setFont(font);
    ceilingSpin->setMinimum(75);
    ceilingSpin->setMaximum(100);
    ceilingSpin->setValue(100);

    gridLayout->addWidget(ceilingSpin, 1, 1, 1, 1);

    ceilingSlider = new QSlider(SupportLevel);
    ceilingSlider->setObjectName(QString::fromUtf8("ceilingSlider"));
    ceilingSlider->setMinimum(7500);
    ceilingSlider->setMaximum(10000);
    ceilingSlider->setPageStep(100);
    ceilingSlider->setValue(10000);
    ceilingSlider->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(ceilingSlider, 1, 2, 1, 1);

    label_5 = new QLabel(SupportLevel);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setFont(font);

    gridLayout->addWidget(label_5, 2, 0, 1, 1);

    blurSpin = new QDoubleSpinBox(SupportLevel);
    blurSpin->setObjectName(QString::fromUtf8("blurSpin"));
    blurSpin->setFont(font);

    gridLayout->addWidget(blurSpin, 2, 1, 1, 1);

    blurSlider = new QSlider(SupportLevel);
    blurSlider->setObjectName(QString::fromUtf8("blurSlider"));
    blurSlider->setMaximum(1000);
    blurSlider->setPageStep(100);
    blurSlider->setValue(300);
    blurSlider->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(blurSlider, 2, 2, 1, 1);


    vboxLayout->addLayout(gridLayout);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    recalculateButton = new QPushButton(SupportLevel);
    recalculateButton->setObjectName(QString::fromUtf8("recalculateButton"));
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(recalculateButton->sizePolicy().hasHeightForWidth());
    recalculateButton->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(recalculateButton);

    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem1);


    vboxLayout->addLayout(hboxLayout);


    retranslateUi(SupportLevel);
    QObject::connect(floorSlider, SIGNAL(valueChanged(int)), floorSpin, SLOT(update()));

    QMetaObject::connectSlotsByName(SupportLevel);
    } // setupUi

    void retranslateUi(QWidget *SupportLevel)
    {
    SupportLevel->setWindowTitle(QApplication::translate("SupportLevel", "Form", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("SupportLevel", "Floor", 0, QApplication::UnicodeUTF8));
    floorSpin->setSuffix(QApplication::translate("SupportLevel", "%", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("SupportLevel", "Ceiling", 0, QApplication::UnicodeUTF8));
    ceilingSpin->setSuffix(QApplication::translate("SupportLevel", "%", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("SupportLevel", "Blur", 0, QApplication::UnicodeUTF8));
    recalculateButton->setText(QApplication::translate("SupportLevel", "Recalculate", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(SupportLevel);
    } // retranslateUi

};

namespace Ui {
    class SupportLevel: public Ui_SupportLevel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUPPORTLEVEL_H
