/********************************************************************************
** Form generated from reading UI file 'enlarge.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ENLARGE_H
#define UI_ENLARGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Enlarge
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;

    void setupUi(QWidget *Enlarge)
    {
        if (Enlarge->objectName().isEmpty())
            Enlarge->setObjectName(QStringLiteral("Enlarge"));
        Enlarge->resize(1000, 640);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Enlarge->sizePolicy().hasHeightForWidth());
        Enlarge->setSizePolicy(sizePolicy);
        Enlarge->setMinimumSize(QSize(340, 200));
        Enlarge->setMaximumSize(QSize(16777215, 16777215));
        Enlarge->setLayoutDirection(Qt::LeftToRight);
        verticalLayout = new QVBoxLayout(Enlarge);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(Enlarge);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setMinimumSize(QSize(320, 180));
        label->setMaximumSize(QSize(16777215, 16777215));
        label->setStyleSheet(QStringLiteral("Background:rgb(0, 0, 0)"));
        label->setScaledContents(false);
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);


        retranslateUi(Enlarge);

        QMetaObject::connectSlotsByName(Enlarge);
    } // setupUi

    void retranslateUi(QWidget *Enlarge)
    {
        Enlarge->setWindowTitle(QApplication::translate("Enlarge", "\346\224\276\345\244\247\346\252\242\350\246\226", 0));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Enlarge: public Ui_Enlarge {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ENLARGE_H
