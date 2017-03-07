#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#define MAX_BUFFER_SIZE 80
// Qt
#include <QtCore/QTime>
#include <QtCore/QThread>
#include <QMutex>
#include <QQueue>
#include <QTcpSocket>
#include <QAbstractSocket>
// OpenCV
#include <opencv2/highgui/highgui.hpp>

//Local
#include "ConcurrentQueue.h"
#include <mainwindow.h>
#include <ProcessThread.h>
#include <VlcThread.h>
#include "outputLog.h"
//C++
#include <iomanip>
#include <iostream>

using namespace std;
using namespace cv;

class ProcessThread;
class MainWindow;

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    CaptureThread(MainWindow* main_window,ConcurrentQueue<Mat> *queue,QString address);
    ~CaptureThread();
    void stop();
    bool connectToCamera(const string stream_addr);
    bool disconnectCamera();
    bool isCameraConnected();
    //Thread's Reference
    ProcessThread *ProcessingThread;

    //Controll thread
    void pause();
    void unpause();
    QString cap_address;
    bool isPause;

protected:
    void run();

signals:
    void PrintMyLog(QString msg,const int level);
    void processedImage(const QImage);

private:      
    QMutex doStopMutex,doPauseMutex;
    QWaitCondition pauseCond;
    ConcurrentQueue<Mat>* frame_queue;
    int queue_counter,empty_counter;
    VideoCapture cap;
    //Check ip alive
    QTcpSocket Qsocket;
//    QTimer *ping_timer;
    int frame_width;
    int frame_height;
    //****Update FPS****
    void updateFPS(int);
    QTime cap_t;
    QQueue<int> fps;
    volatile bool doStop;
    int captureTime;
    int sampleNumber;
    int fpsSum;
    int averageFPS;
    //******************
    //用來印在程式的TextList上的變數
    QString Log_Msg;  
    Mat src_frame;//讀到的原始影像
    Mat show_frame;//顯示在UI上的Frame
    Mat push_frame;//Push進Buffer的Frame
    //Covert Mat to QImage
    QImage Mat2QImage(Mat const& src);
    //Output the Log Messages
//    void Output_Log(const string& messages);
    //Log File Name
    string log_filename;
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
    outputLog *myLog;
};

#endif // CAPTURETHREAD_H
