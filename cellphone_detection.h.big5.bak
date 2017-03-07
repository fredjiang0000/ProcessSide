#ifndef CELLPHONE_DETECTION
#define CELLPHONE_DETECTION
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include <Qtcore/QThread>
#include <QMutex>
///////////////
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "bgsubmog.h"
#include "detection.h"
#include "display.h"
#include "objtrack.h"
#include "skeleton.h"
#include <time.h>
////////////////
#endif // CELLPHONE_DECTECTION
using namespace cv;

class cellphone_detection : public QThread{
public:
    cellphone_detection(Mat *frame,int *detect_phone_sensitivity);
    ~cellphone_detection();
    void stop();
protected:
    void run();
private:
//    void CellPhoneDetection(vector<Mat> pick_Img);
    Vector<Rect> BG_Skin_mask(Mat frame);
    Mat *frame;
    int* detect_phone_sensitivity;
    QMutex doStopMutex;
    volatile bool doStop;
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
    outputLog *myLog;
    //回傳檔案路徑(檔案名設為現在時間加上一個計數)
    string saveFilePath(const int count);
};


