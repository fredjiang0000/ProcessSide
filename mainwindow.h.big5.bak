#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//QT
#include <QMainWindow>
#include <QTime>
#include <QPainter>
#include <QPen>
#include <QMessageBox>
//OpenCV
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
//C/C++
#include <iostream>
//Local
#include "enlarge.h"
#include "connection.h"
#include "ProcessThread.h"
#include "smtp.h"
#include "outputLog.h"



using namespace cv;
using namespace std;

class connection;
class Enlarge;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    connection *F_connection;
    void setOpenEnlarge(bool flag);
protected:
    void closeEvent(QCloseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *e);
//    void mouseDoubleClickEvent(QMouseEvent *e);
signals:
    void enlarge_update(const QImage& img);

private slots:
    void updatePlayerUI(const QImage img);
    void UpdateMyLog(QString msg,const int level);
    void on_pushButton_connection_clicked();
    void on_pushButton_enlarge_clicked();

private:
    Ui::MainWindow *ui;
//    summarization fSum;
    Enlarge *F_enlarge;
    QImage qimg;
    Mat src_img;
    Mutex mutex;
    int empty_counter;
    bool open_enlarge;
};

#endif // MAINWINDOW_H
