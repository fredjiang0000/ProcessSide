#ifndef OBJTRACK_H
#define OBJTRACK_H

#include "bgsubmog.h"
#include "objdata.h"
#include "outputLog.h"

class objTrack
{
public:
    objTrack(outputLog *mylog);
    objTrack(float whR,outputLog *mylog);
    objTrack(int minSizem,outputLog *mylog);
    objTrack(float whR,int minSize,outputLog *mylog);
    ~objTrack();

    vector<Rect> getObj();
    vector<Rect> getGroup();
    bool tracker(Mat frame);
    Mat binaryImage();

    int Get_HumanNum(Rect obj);

private:
    int minObjSize;
    float whRatio;
    int errTime = 3;
    bgSubMog  bgProcessor;
    vector<objData> objList;
//    vector<Rect> obj;
    bool tracking = false;
    bool FrameErrFlag = false;
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
    outputLog *myLog;
};

#endif // OBJTRACK_H
