/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox_2;
    QPushButton *pushButton_enlarge;
    QGroupBox *groupBox;
    QPushButton *pushButton_connection;
    QLabel *hint;
    QFrame *frame_1;
    QLabel *label_1;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QTextEdit *LogEdit;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(877, 691);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(500, 500));
        MainWindow->setMaximumSize(QSize(20000, 20000));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy1);
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(660, 140, 200, 121));
        sizePolicy.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy);
        groupBox_2->setMinimumSize(QSize(200, 100));
        groupBox_2->setMaximumSize(QSize(200, 200));
        QFont font;
        font.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        groupBox_2->setFont(font);
        pushButton_enlarge = new QPushButton(groupBox_2);
        pushButton_enlarge->setObjectName(QStringLiteral("pushButton_enlarge"));
        pushButton_enlarge->setGeometry(QRect(30, 40, 131, 51));
        pushButton_enlarge->setCursor(QCursor(Qt::PointingHandCursor));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(660, 10, 200, 121));
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        groupBox->setMinimumSize(QSize(200, 100));
        groupBox->setMaximumSize(QSize(200, 200));
        groupBox->setFont(font);
        pushButton_connection = new QPushButton(groupBox);
        pushButton_connection->setObjectName(QStringLiteral("pushButton_connection"));
        pushButton_connection->setGeometry(QRect(30, 40, 131, 51));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font1.setPointSize(12);
        pushButton_connection->setFont(font1);
        pushButton_connection->setCursor(QCursor(Qt::PointingHandCursor));
        hint = new QLabel(centralWidget);
        hint->setObjectName(QStringLiteral("hint"));
        hint->setGeometry(QRect(270, 610, 320, 30));
        sizePolicy.setHeightForWidth(hint->sizePolicy().hasHeightForWidth());
        hint->setSizePolicy(sizePolicy);
        hint->setMinimumSize(QSize(320, 30));
        QFont font2;
        font2.setFamily(QString::fromUtf8("\346\200\235\346\272\220\351\273\221\351\253\224 TWHK Medium"));
        font2.setPointSize(14);
        font2.setBold(true);
        font2.setWeight(75);
        hint->setFont(font2);
        hint->setStyleSheet(QStringLiteral("color:blue"));
        hint->setAlignment(Qt::AlignCenter);
        frame_1 = new QFrame(centralWidget);
        frame_1->setObjectName(QStringLiteral("frame_1"));
        frame_1->setGeometry(QRect(10, 10, 640, 360));
        frame_1->setFrameShape(QFrame::StyledPanel);
        frame_1->setFrameShadow(QFrame::Raised);
        label_1 = new QLabel(frame_1);
        label_1->setObjectName(QStringLiteral("label_1"));
        label_1->setGeometry(QRect(0, 0, 640, 360));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_1->sizePolicy().hasHeightForWidth());
        label_1->setSizePolicy(sizePolicy2);
        label_1->setMinimumSize(QSize(320, 180));
        label_1->setMaximumSize(QSize(640, 360));
        label_1->setFocusPolicy(Qt::NoFocus);
        label_1->setStyleSheet(QStringLiteral("Background-color: #000;"));
        label_1->setScaledContents(false);
        label_1->setAlignment(Qt::AlignCenter);
        scrollArea = new QScrollArea(centralWidget);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setGeometry(QRect(10, 380, 851, 221));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 849, 219));
        LogEdit = new QTextEdit(scrollAreaWidgetContents);
        LogEdit->setObjectName(QStringLiteral("LogEdit"));
        LogEdit->setGeometry(QRect(0, 0, 851, 221));
        QFont font3;
        font3.setFamily(QString::fromUtf8("\345\276\256\350\273\237\346\255\243\351\273\221\351\253\224"));
        font3.setPointSize(11);
        font3.setBold(true);
        font3.setWeight(75);
        LogEdit->setFont(font3);
        LogEdit->setContextMenuPolicy(Qt::DefaultContextMenu);
        LogEdit->setUndoRedoEnabled(true);
        LogEdit->setReadOnly(true);
        LogEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
        scrollArea->setWidget(scrollAreaWidgetContents);
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 877, 22));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "\347\233\243\350\246\226\347\263\273\347\265\261-\350\231\225\347\220\206\347\253\257", 0));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "\347\225\253\351\235\242", 0));
        pushButton_enlarge->setText(QApplication::translate("MainWindow", "\346\224\276\345\244\247\346\252\242\350\246\226", 0));
        groupBox->setTitle(QApplication::translate("MainWindow", "\351\200\243\347\267\232", 0));
        pushButton_connection->setText(QApplication::translate("MainWindow", "\351\200\243\347\267\232\350\250\255\345\256\232", 0));
        hint->setText(QString());
        label_1->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
