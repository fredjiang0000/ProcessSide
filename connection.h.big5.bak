#ifndef CONNECTION_H
#define CONNECTION_H

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#else
#include <time.h>
#endif

//QT
#include <QtCore/QTime>
#include <QWidget>

//Local
#include "mainwindow.h"
#include "ProcessThread.h"
#include "CaptureThread.h"
#include "VlcThread.h"
#include "outputLog.h"

//C++
#include <iostream>
#include<fstream>
using namespace std;

namespace Ui {
class connection;
}
class MainWindow;
class ProcessThread;
class CaptureThread;
class VlcThread;

class connection : public QWidget
{
    Q_OBJECT

public:
    explicit connection(MainWindow* main_window,QWidget *parent = 0);
    ~connection();
    //Stop the Threads & 重新開啟Thread並連線至顯式端
    bool Reconnection();
    //Stop the Threads
    void stopAllThreads();
    void stopCaptureThread();
    void stopProcessingThread();
    void stopVLCThread();
    bool isCamDisconnection();
    //Delete the Threads
    void delAllThreads();
    //VLC、Push、Pop thread
    ProcessThread *process_thread;
    CaptureThread *cap_thread;
    VlcThread *vlc_thread;
protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent * event);

signals:
    void PrintMyLog(QString msg,const int level);
private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_ok_clicked();
    void isIPAddressAlive();

private:
    Ui::connection *ui;
    MainWindow *main_window;
    //監視器的解析度(傳參數至ProcessThread)
    int frame_width,frame_height;
    //監視器的FPS(傳參數至ProcessThread)
    int Cam_FPS;
    //Frame Buffer
    ConcurrentQueue<Mat> *concurrent_queue;
    QString Log_Msg;
    //保留"連線設定"欄位的資料(共四個)
    string stream_addr;//監視器影像串流網址
    QByteArray server_ip_array,location_array;//傳遞 顯示端IP 和 監視器地點 用(char*型態)
    int server_port;//顯示端的Port
    //每幾秒中檢查監視器是否連線正常
    QTcpSocket Qsocket;
    QTimer *ping_timer;
    QString ping_cam_ip;
    int ping_cam_port;
    bool isCamDisconnect;
    //印出Log訊息到TXT檔
    void Output_Log(const string messages);
    //從監視器影像串流網址切出IP+Port
    QString Get_Cam_IP_Port(QString rtsp_address);
    //從IP+Port中切出IP
    QString Get_Cam_IP(QString cam_ip_port);
    //從IP+Port中切出Port
    QString Get_Cam_Port(QString cam_ip_port);
    //檢查監視器的IP是否Ping的到
    bool isCameraIPAlive(const QString ping_ip, const int port);
    //檢查監視器的串流網址是否能打開,成功打開則Set frame_width&height&Cam_FPS
    bool connectToCamera(VideoCapture &cap, const string stream_addr);
    //是否為影片檔案
    bool openVideo;
    //讀取Network_profiles.txt中的連線設定參數
    void Read_Network_profiles();
    //產生Thread和初始化(尚未定義)
    void createProcessThread(VideoCapture &cap, MainWindow main_win, ConcurrentQueue<Mat> *frame_queue);
    void createCaptureThread();
    void createVlcThread();
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
    outputLog *myLog;
};

#endif // CONNECTION_H
