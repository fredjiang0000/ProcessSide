#ifndef CHECK_THREAD
#define CHECK_THREAD
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include <QThread>
#include <QObject>
#include <QMutex>
#include "stdafx.h"
#include "header.h"

using namespace cv;
using namespace std;
class check_thread : public QThread{
    Q_OBJECT
public:
    check_thread();
    void mutex_push(Mat frame, Rect rect);
    void Set_result_ROI(bool bottom);
    bool GetRoi(Rect &roi);
    void stop();
    bool result_ROI;
    bool result;
    ~check_thread();
protected:
    void run();
private:
    Rect temp_rect;
    bool doStop;
    vector<Mat> srcFrame;
    vector<Rect> srcRect;
    IplImage *Mat2Ipl(Mat img);
    QMutex mutex,doStopMutex,ROI_mutex;
    void mutex_erase();
    bool checkError(IplImage* input);
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
    //回傳檔案路徑(檔案名設為現在時間加上一個計數)
    string saveFilePath(const int count);
};
#endif // CHECK_THREAD
