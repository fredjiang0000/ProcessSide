/*
    Author:  沈思成、劉佳盈、林瑞嚴、莊弼堯、蔡安迪
    Date:    2016/1/123
    Version: 3.8.0
    Purpose: 接收IP Camera的影像串流傳送給顯示端，
             等待顯示端下指令做相對應的影像處理功能，
             功能包括使用手機辨識和指定區域物體移動的判斷，
             發生異常可以傳送警報和Log訊息給顯示端
*/
#include <winsock2.h>
#include "mainwindow.h"
#include <QApplication>
#include "ImgProcess.h"
#include "widget.h"
#include <QTextCodec>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

CvMemStorage* storage = NULL;

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    storage = cvCreateMemStorage(0);
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
//        cellDetect_storage = cvCreateMemStorage(0);
        //WAN IP address : rtsp://140.117.168.125:5002/medias1 (anthoer port:5003)
        //LAN IP address : rtsp://192.168.1.14:554/medias1 (anthoer IP:192.168.1.8)
        //Local IP :  rtsp://169.254.60.101:554/medias1

        return a.exec();
}

