#ifndef OBJDATA_H
#define OBJDATA_H

#include <iostream>

#include <opencv2/opencv.hpp>

#include <vector>


using namespace std;

using namespace cv;

class objData
{
public:
    objData();
    ~objData();

    bool isNew(){return isNewFlag;}
    void setNew(bool a){isNewFlag = a;return;}
    bool isGroup(){return isGroupFlag;}
    void setGroup(bool a){isGroupFlag = a;return;}

    void setObjRoi(Rect loc){objRoi = loc; --updateTime; return;}
    Rect getObjRoi(){return objRoi;}

    int getUpdateTime(){return updateTime;}
    int Human_num=0;



private:
    bool mktFlag = false;
    bool isNewFlag = false;
    bool isGroupFlag = false;
    int updateTime = 5;

    Rect objRoi;

};

#endif // OBJDATA_H

