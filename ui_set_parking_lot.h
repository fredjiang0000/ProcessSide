/********************************************************************************
** Form generated from reading UI file 'set_parking_lot.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SET_PARKING_LOT_H
#define UI_SET_PARKING_LOT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <my_qlabel.h>

QT_BEGIN_NAMESPACE

class Ui_Set_Parking_Lot
{
public:
    my_qlabel *label;
    QLabel *label_2;
    QPushButton *pushButton_clear_all;
    QPushButton *pushButton_ok;
    QPushButton *pushButton_cancel;
    QLabel *label_pos;
    QLabel *label_event;
    QPushButton *pushButton_clear_last;

    void setupUi(QWidget *Set_Parking_Lot)
    {
        if (Set_Parking_Lot->objectName().isEmpty())
            Set_Parking_Lot->setObjectName(QStringLiteral("Set_Parking_Lot"));
        Set_Parking_Lot->setEnabled(true);
        Set_Parking_Lot->resize(1070, 673);
        label = new my_qlabel(Set_Parking_Lot);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 50, 800, 600));
        label->setMouseTracking(true);
        label->setStyleSheet(QStringLiteral("background-color: rgb(0, 0, 0);"));
        label->setScaledContents(false);
        label_2 = new QLabel(Set_Parking_Lot);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(270, 10, 371, 31));
        QFont font;
        font.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font.setPointSize(14);
        font.setBold(true);
        font.setWeight(75);
        label_2->setFont(font);
        label_2->setStyleSheet(QStringLiteral("color:rgb(0, 0, 255)"));
        pushButton_clear_all = new QPushButton(Set_Parking_Lot);
        pushButton_clear_all->setObjectName(QStringLiteral("pushButton_clear_all"));
        pushButton_clear_all->setGeometry(QRect(870, 50, 151, 51));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font1.setPointSize(12);
        font1.setBold(true);
        font1.setWeight(75);
        pushButton_clear_all->setFont(font1);
        pushButton_ok = new QPushButton(Set_Parking_Lot);
        pushButton_ok->setObjectName(QStringLiteral("pushButton_ok"));
        pushButton_ok->setGeometry(QRect(850, 220, 91, 41));
        pushButton_ok->setFont(font1);
        pushButton_cancel = new QPushButton(Set_Parking_Lot);
        pushButton_cancel->setObjectName(QStringLiteral("pushButton_cancel"));
        pushButton_cancel->setGeometry(QRect(960, 220, 91, 41));
        pushButton_cancel->setFont(font1);
        label_pos = new QLabel(Set_Parking_Lot);
        label_pos->setObjectName(QStringLiteral("label_pos"));
        label_pos->setGeometry(QRect(880, 530, 141, 51));
        label_event = new QLabel(Set_Parking_Lot);
        label_event->setObjectName(QStringLiteral("label_event"));
        label_event->setGeometry(QRect(880, 600, 111, 51));
        pushButton_clear_last = new QPushButton(Set_Parking_Lot);
        pushButton_clear_last->setObjectName(QStringLiteral("pushButton_clear_last"));
        pushButton_clear_last->setGeometry(QRect(870, 120, 151, 51));
        pushButton_clear_last->setFont(font1);

        retranslateUi(Set_Parking_Lot);

        QMetaObject::connectSlotsByName(Set_Parking_Lot);
    } // setupUi

    void retranslateUi(QWidget *Set_Parking_Lot)
    {
        Set_Parking_Lot->setWindowTitle(QApplication::translate("Set_Parking_Lot", "\350\250\255\345\256\232\345\201\265\346\270\254\345\201\234\350\273\212\346\240\274", 0));
        label->setText(QString());
        label_2->setText(QApplication::translate("Set_Parking_Lot", "\350\250\255\345\256\232\350\246\201\345\201\265\346\270\254\347\232\204\345\201\234\350\273\212\344\275\215\347\257\204\345\234\215", 0));
        pushButton_clear_all->setText(QApplication::translate("Set_Parking_Lot", "\346\270\205\351\231\244\345\205\250\351\203\250", 0));
        pushButton_ok->setText(QApplication::translate("Set_Parking_Lot", "\347\242\272\345\256\232", 0));
        pushButton_cancel->setText(QApplication::translate("Set_Parking_Lot", "\345\217\226\346\266\210", 0));
        label_pos->setText(QApplication::translate("Set_Parking_Lot", "TextLabel", 0));
        label_event->setText(QApplication::translate("Set_Parking_Lot", "TextLabel", 0));
        pushButton_clear_last->setText(QApplication::translate("Set_Parking_Lot", "\346\270\205\351\231\244\344\270\212\344\270\200\345\200\213", 0));
    } // retranslateUi

};

namespace Ui {
    class Set_Parking_Lot: public Ui_Set_Parking_Lot {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SET_PARKING_LOT_H
