/********************************************************************************
** Form generated from reading UI file 'smtp.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SMTP_H
#define UI_SMTP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_smtp
{
public:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QCheckBox *checkBox;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QLineEdit *lineEdit_4;
    QPushButton *pushButton_ok;
    QPushButton *pushButton_cancel;

    void setupUi(QWidget *smtp)
    {
        if (smtp->objectName().isEmpty())
            smtp->setObjectName(QStringLiteral("smtp"));
        smtp->resize(543, 331);
        QFont font;
        font.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        smtp->setFont(font);
        label = new QLabel(smtp);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(30, 60, 181, 61));
        QFont font1;
        font1.setPointSize(13);
        label->setFont(font1);
        label_2 = new QLabel(smtp);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(30, 110, 171, 61));
        label_2->setFont(font1);
        label_3 = new QLabel(smtp);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(30, 160, 181, 61));
        label_3->setFont(font1);
        label_4 = new QLabel(smtp);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(30, 210, 201, 61));
        label_4->setFont(font1);
        checkBox = new QCheckBox(smtp);
        checkBox->setObjectName(QStringLiteral("checkBox"));
        checkBox->setGeometry(QRect(150, 20, 231, 41));
        checkBox->setFont(font1);
        lineEdit = new QLineEdit(smtp);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(240, 80, 271, 27));
        lineEdit_2 = new QLineEdit(smtp);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(240, 130, 271, 27));
        lineEdit_3 = new QLineEdit(smtp);
        lineEdit_3->setObjectName(QStringLiteral("lineEdit_3"));
        lineEdit_3->setGeometry(QRect(240, 180, 271, 27));
        lineEdit_3->setEchoMode(QLineEdit::Password);
        lineEdit_4 = new QLineEdit(smtp);
        lineEdit_4->setObjectName(QStringLiteral("lineEdit_4"));
        lineEdit_4->setGeometry(QRect(240, 230, 271, 27));
        pushButton_ok = new QPushButton(smtp);
        pushButton_ok->setObjectName(QStringLiteral("pushButton_ok"));
        pushButton_ok->setGeometry(QRect(120, 270, 111, 41));
        pushButton_cancel = new QPushButton(smtp);
        pushButton_cancel->setObjectName(QStringLiteral("pushButton_cancel"));
        pushButton_cancel->setGeometry(QRect(270, 270, 121, 41));

        retranslateUi(smtp);

        QMetaObject::connectSlotsByName(smtp);
    } // setupUi

    void retranslateUi(QWidget *smtp)
    {
        smtp->setWindowTitle(QApplication::translate("smtp", "\351\200\232\345\240\261\350\250\255\345\256\232", 0));
        label->setText(QApplication::translate("smtp", "SMTP\344\274\272\346\234\215\345\231\250\357\274\232", 0));
        label_2->setText(QApplication::translate("smtp", "\344\275\277\347\224\250\350\200\205\345\220\215\347\250\261\357\274\232", 0));
        label_3->setText(QApplication::translate("smtp", "\345\257\206\347\242\274\357\274\232", 0));
        label_4->setText(QApplication::translate("smtp", "\345\257\204\344\273\266\350\200\205\351\203\265\344\273\266\344\275\215\345\235\200\357\274\232", 0));
        checkBox->setText(QApplication::translate("smtp", "\351\226\213\345\225\237E-mail\351\200\232\347\237\245", 0));
        pushButton_ok->setText(QApplication::translate("smtp", "\347\242\272\345\256\232", 0));
        pushButton_cancel->setText(QApplication::translate("smtp", "\345\217\226\346\266\210", 0));
    } // retranslateUi

};

namespace Ui {
    class smtp: public Ui_smtp {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SMTP_H
