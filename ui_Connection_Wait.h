/********************************************************************************
** Form generated from reading UI file 'Connection_Wait.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONNECTION_WAIT_H
#define UI_CONNECTION_WAIT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Connection_Wait
{
public:
    QGroupBox *groupBox;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QPushButton *pushButton_ok;
    QPushButton *pushButton_cancel;

    void setupUi(QWidget *Connection_Wait)
    {
        if (Connection_Wait->objectName().isEmpty())
            Connection_Wait->setObjectName(QStringLiteral("Connection_Wait"));
        Connection_Wait->resize(317, 240);
        groupBox = new QGroupBox(Connection_Wait);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(20, 20, 251, 131));
        QFont font;
        font.setFamily(QStringLiteral("Consolas"));
        font.setPointSize(14);
        font.setBold(true);
        font.setWeight(75);
        groupBox->setFont(font);
        widget = new QWidget(groupBox);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(20, 40, 203, 58));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(widget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setFont(font);

        verticalLayout->addWidget(label_2);

        lineEdit = new QLineEdit(widget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
        lineEdit->setSizePolicy(sizePolicy);
        lineEdit->setMaximumSize(QSize(100, 16777215));

        verticalLayout->addWidget(lineEdit);

        pushButton_ok = new QPushButton(Connection_Wait);
        pushButton_ok->setObjectName(QStringLiteral("pushButton_ok"));
        pushButton_ok->setGeometry(QRect(20, 170, 101, 41));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\273\237\346\255\243\351\273\221\351\253\224"));
        font1.setPointSize(14);
        font1.setBold(true);
        font1.setWeight(75);
        pushButton_ok->setFont(font1);
        pushButton_cancel = new QPushButton(Connection_Wait);
        pushButton_cancel->setObjectName(QStringLiteral("pushButton_cancel"));
        pushButton_cancel->setGeometry(QRect(170, 170, 101, 41));
        pushButton_cancel->setFont(font1);

        retranslateUi(Connection_Wait);

        QMetaObject::connectSlotsByName(Connection_Wait);
    } // setupUi

    void retranslateUi(QWidget *Connection_Wait)
    {
        Connection_Wait->setWindowTitle(QApplication::translate("Connection_Wait", "Form", 0));
        groupBox->setTitle(QApplication::translate("Connection_Wait", "\351\200\243\347\267\232\350\250\255\345\256\232", 0));
        label_2->setText(QApplication::translate("Connection_Wait", "Listen Port ", 0));
        pushButton_ok->setText(QApplication::translate("Connection_Wait", "\347\242\272\345\256\232", 0));
        pushButton_cancel->setText(QApplication::translate("Connection_Wait", "\345\217\226\346\266\210", 0));
    } // retranslateUi

};

namespace Ui {
    class Connection_Wait: public Ui_Connection_Wait {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONNECTION_WAIT_H
