#ifndef VlcThread_H
#define VlcThread_H

//Windows
#include <windows.h>
// Qt
#include <QtCore/QTime>
#include <QtCore/QThread>
#include <QMutex>
// OpenCV
#include <opencv2/highgui/highgui.hpp>
// VLC
#include "vlc\libvlc.h"
#include "vlc\vlc.h"
//C&C++
#include <iostream>
//Local
#include "ConcurrentQueue.h"
#include "mainwindow.h"

using namespace std;
using namespace cv;

class MainWindow;

struct ctx
{
   Mat* image;
   HANDLE mutex;
   uchar *pixels;
};
//********************* VLC API **************************
//Purpose : Get the Frame Data and convert it to the Mat
void *lock( void *data, void**p_pixels );
void display( void *data, void *id );
void unlock( void *data, void *id, void *const *p_pixels );
//********************************************************

class VlcThread : public QThread
{
    Q_OBJECT
public:
    VlcThread(MainWindow* main_window,ConcurrentQueue<Mat> *queue, const bool openVideo);
    ~VlcThread();
    //VLC Open Streaming and Initialization
    void VLC_Streaming_Init(const char* open_streaming_addr,int frame_width, int frame_height);
    //Stop and Release the VLC_Media_Player
    void stop();
protected:
    void run();
signals:
    void processedImage(const QImage);
private:
    //Frame Buufer
    ConcurrentQueue<Mat> *frame_queue;
    //Control thread flag
    bool doStop;
    QMutex doStopMutex;
    //********************* VLC Media Player *****************
    libvlc_instance_t  *vlcInstance;
    libvlc_media_player_t *mp;
    libvlc_media_t   *media;
    //********************************************************
    //Covert Mat to QImage
    QImage Mat2QImage(Mat const& src);
    //是否開啟影片檔
    bool openVideo;
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);

};

#endif // VlcThread_H
