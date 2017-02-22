/********************************************************************************
** Form generated from reading UI file 'background_display.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BACKGROUND_DISPLAY_H
#define UI_BACKGROUND_DISPLAY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Background_Display
{
public:
    QLabel *label;
    QPushButton *pushButton_close;

    void setupUi(QWidget *Background_Display)
    {
        if (Background_Display->objectName().isEmpty())
            Background_Display->setObjectName(QStringLiteral("Background_Display"));
        Background_Display->resize(1002, 641);
        label = new QLabel(Background_Display);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 20, 960, 540));
        label->setStyleSheet(QStringLiteral("background-color: rgb(0, 0, 0);"));
        label->setScaledContents(true);
        pushButton_close = new QPushButton(Background_Display);
        pushButton_close->setObjectName(QStringLiteral("pushButton_close"));
        pushButton_close->setGeometry(QRect(390, 580, 171, 51));
        QFont font;
        font.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        pushButton_close->setFont(font);

        retranslateUi(Background_Display);

        QMetaObject::connectSlotsByName(Background_Display);
    } // setupUi

    void retranslateUi(QWidget *Background_Display)
    {
        Background_Display->setWindowTitle(QApplication::translate("Background_Display", "\344\271\213\345\211\215\347\232\204\350\203\214\346\231\257\345\234\226", 0));
        label->setText(QApplication::translate("Background_Display", "TextLabel", 0));
        pushButton_close->setText(QApplication::translate("Background_Display", "\351\227\234\351\226\211", 0));
    } // retranslateUi

};

namespace Ui {
    class Background_Display: public Ui_Background_Display {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BACKGROUND_DISPLAY_H
