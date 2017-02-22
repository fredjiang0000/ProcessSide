/********************************************************************************
** Form generated from reading UI file 'connection.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONNECTION_H
#define UI_CONNECTION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_connection
{
public:
    QGroupBox *groupBox;
    QLabel *label;
    QLabel *hint;
    QLineEdit *stream_address;
    QLabel *label_2;
    QLineEdit *server_ip;
    QLabel *label_3;
    QLineEdit *server_port;
    QLabel *label_4;
    QLineEdit *cam_location;
    QPushButton *pushButton_ok;
    QPushButton *pushButton_cancel;

    void setupUi(QWidget *connection)
    {
        if (connection->objectName().isEmpty())
            connection->setObjectName(QStringLiteral("connection"));
        connection->resize(435, 451);
        QFont font;
        font.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        connection->setFont(font);
        groupBox = new QGroupBox(connection);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(30, 10, 381, 351));
        QFont font1;
        font1.setPointSize(14);
        groupBox->setFont(font1);
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 110, 281, 31));
        QFont font2;
        font2.setFamily(QStringLiteral("Consolas"));
        font2.setPointSize(14);
        label->setFont(font2);
        hint = new QLabel(groupBox);
        hint->setObjectName(QStringLiteral("hint"));
        hint->setGeometry(QRect(160, 300, 201, 31));
        hint->setFont(font2);
        stream_address = new QLineEdit(groupBox);
        stream_address->setObjectName(QStringLiteral("stream_address"));
        stream_address->setGeometry(QRect(20, 140, 261, 27));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 190, 341, 31));
        label_2->setFont(font2);
        server_ip = new QLineEdit(groupBox);
        server_ip->setObjectName(QStringLiteral("server_ip"));
        server_ip->setGeometry(QRect(20, 220, 261, 27));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(20, 270, 291, 31));
        label_3->setFont(font2);
        server_port = new QLineEdit(groupBox);
        server_port->setObjectName(QStringLiteral("server_port"));
        server_port->setGeometry(QRect(20, 300, 81, 27));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(20, 40, 281, 31));
        label_4->setFont(font2);
        cam_location = new QLineEdit(groupBox);
        cam_location->setObjectName(QStringLiteral("cam_location"));
        cam_location->setGeometry(QRect(20, 70, 261, 27));
        pushButton_ok = new QPushButton(connection);
        pushButton_ok->setObjectName(QStringLiteral("pushButton_ok"));
        pushButton_ok->setGeometry(QRect(30, 380, 145, 50));
        pushButton_cancel = new QPushButton(connection);
        pushButton_cancel->setObjectName(QStringLiteral("pushButton_cancel"));
        pushButton_cancel->setGeometry(QRect(230, 380, 145, 50));

        retranslateUi(connection);

        QMetaObject::connectSlotsByName(connection);
    } // setupUi

    void retranslateUi(QWidget *connection)
    {
        connection->setWindowTitle(QApplication::translate("connection", "\351\200\243\347\267\232\350\250\255\345\256\232", 0));
        groupBox->setTitle(QApplication::translate("connection", "\351\200\243\347\267\232\350\250\255\345\256\232", 0));
        label->setText(QApplication::translate("connection", "\347\233\243\350\246\226\345\231\250\347\232\204\345\275\261\345\203\217\344\270\262\346\265\201\347\266\262\345\235\200", 0));
        hint->setText(QApplication::translate("connection", "hint", 0));
        label_2->setText(QApplication::translate("connection", "\351\241\257\347\244\272\347\253\257\347\232\204IP\344\275\215\345\235\200", 0));
        label_3->setText(QApplication::translate("connection", "\351\241\257\347\244\272\347\253\257\347\232\204Port", 0));
        label_4->setText(QApplication::translate("connection", "\347\233\243\350\246\226\345\231\250\345\234\260\351\273\236", 0));
        pushButton_ok->setText(QApplication::translate("connection", "\347\242\272\345\256\232", 0));
        pushButton_cancel->setText(QApplication::translate("connection", "\345\217\226\346\266\210", 0));
    } // retranslateUi

};

namespace Ui {
    class connection: public Ui_connection {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONNECTION_H
