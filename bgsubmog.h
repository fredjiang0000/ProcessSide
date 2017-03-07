#ifndef BGSUBMOG_H
#define BGSUBMOG_H


#include <iostream>

#include <opencv2/opencv.hpp>

#include <math.h>

#include <vector>

#include"outputLog.h"

using namespace std;

using namespace cv;


class bgSubMog
{
public:
    float learnRate;
    int frame_lost_time;
    bgSubMog();
    bgSubMog(outputLog *mylog);
    bgSubMog(BackgroundSubtractorMOG mog,outputLog *mylog);
    ~bgSubMog();
    Mat getMask();
    vector<Rect> objTrack(const Mat frame,int min_obj_size);
    Mat bgSubtractor(const Mat frame);

private:
    Mat forgroundMask;
    BackgroundSubtractorMOG bgmog;
    void dilateErode(Mat* srcImg,Mat* dstImg,Mat element,int iteration);
    vector<Rect> connectComponent(const Mat mask, int min_obj_size);
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
    outputLog *myLog;
};

#endif // BGSUBMOG_H
