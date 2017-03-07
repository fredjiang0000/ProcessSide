/********************************************************************************
** Form generated from reading UI file 'summarization.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUMMARIZATION_H
#define UI_SUMMARIZATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_summarization
{
public:
    QDialogButtonBox *buttonBox;
    QGraphicsView *graphicsView;
    QGroupBox *groupBox;
    QLabel *label;
    QPushButton *pushButton;
    QGroupBox *groupBox_2;
    QDateTimeEdit *dateTimeEdit;
    QDateTimeEdit *dateTimeEdit_2;
    QLabel *label_2;
    QLabel *label_3;

    void setupUi(QDialog *summarization)
    {
        if (summarization->objectName().isEmpty())
            summarization->setObjectName(QStringLiteral("summarization"));
        summarization->resize(843, 417);
        buttonBox = new QDialogButtonBox(summarization);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(590, 320, 211, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        graphicsView = new QGraphicsView(summarization);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));
        graphicsView->setGeometry(QRect(20, 30, 541, 341));
        groupBox = new QGroupBox(summarization);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(580, 50, 251, 101));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(70, 30, 111, 16));
        pushButton = new QPushButton(groupBox);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(80, 60, 93, 28));
        groupBox_2 = new QGroupBox(summarization);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(570, 170, 261, 111));
        dateTimeEdit = new QDateTimeEdit(groupBox_2);
        dateTimeEdit->setObjectName(QStringLiteral("dateTimeEdit"));
        dateTimeEdit->setGeometry(QRect(50, 30, 194, 22));
        dateTimeEdit->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));
        dateTimeEdit_2 = new QDateTimeEdit(groupBox_2);
        dateTimeEdit_2->setObjectName(QStringLiteral("dateTimeEdit_2"));
        dateTimeEdit_2->setGeometry(QRect(50, 60, 194, 22));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 50, 51, 16));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 80, 41, 20));

        retranslateUi(summarization);
        QObject::connect(buttonBox, SIGNAL(accepted()), summarization, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), summarization, SLOT(reject()));

        QMetaObject::connectSlotsByName(summarization);
    } // setupUi

    void retranslateUi(QDialog *summarization)
    {
        summarization->setWindowTitle(QApplication::translate("summarization", "Dialog", 0));
        groupBox->setTitle(QApplication::translate("summarization", "Region of Interest", 0));
        label->setText(QApplication::translate("summarization", "(XX,YY,WW,HH)", 0));
        pushButton->setText(QApplication::translate("summarization", "Reset", 0));
        groupBox_2->setTitle(QApplication::translate("summarization", "Time", 0));
        label_2->setText(QApplication::translate("summarization", "From:", 0));
        label_3->setText(QApplication::translate("summarization", "    To:", 0));
    } // retranslateUi

};

namespace Ui {
    class summarization: public Ui_summarization {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUMMARIZATION_H
