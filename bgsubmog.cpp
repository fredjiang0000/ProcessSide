/*******************************************
Function name : parameters
= Function Features
+ Function Practice
********************************************/

#include "bgsubmog.h"

#include <iostream>

#include <opencv2/opencv.hpp>

#include <math.h>

#include <vector>

using namespace std;

using namespace cv;


/*******************************************
bgSubMog : BackgroundSubtractorMOG mog
= BackgroundSubtractorMOG Initial
********************************************/
bgSubMog::bgSubMog():learnRate(0.05),frame_lost_time(30)
{
}

bgSubMog::bgSubMog(outputLog *mylog):learnRate(0.05),frame_lost_time(30)
{
    this->myLog = mylog;
    this->myLog->printLog(logAddMem("bgSubMog::bgSubMog(2)"));
}

bgSubMog::bgSubMog(BackgroundSubtractorMOG mog,outputLog *mylog): bgmog(mog) , learnRate(0.05),frame_lost_time(30)
{
    this->myLog = mylog;
    this->myLog->printLog(logAddMem("bgSubMog::bgSubMog(3)"));
}


bgSubMog::~bgSubMog()
{
    myLog->printLog(logAddMem("bgSubMog::~bgSubMog()"));
}
/*******************************************
bgSubstractor : input Image
= Use BackgroundSubtractorMOG to background subtraction
+ chage RGB Image to gray
+ use median filter to delete noice
+ use BackgroundSubtractorMOG get foreground
+ call dilate and Erode iteration to get better foreground
********************************************/
Mat bgSubMog::bgSubtractor(const Mat frame)
{
    this->myLog->printLog(logAddMem("bgSubMog::bgSubtractor() enter"));
//    Mat grayFrame;
//    Mat smooth;
//    cvtColor(frame,grayFrame,CV_RGB2GRAY);
//    medianBlur(grayFrame,smooth,5);

//    GaussianBlur( frame, smooth, Size( 5, 5 ), 0, 0 );

    Mat foreground;
    bgmog(frame,foreground,learnRate);

    Mat element(50,30,CV_8U,Scalar(1));
    dilateErode(&foreground,&foreground,element,1);

    this->myLog->printLog(logAddMem("bgSubMog::bgSubtractor() exit"));
    return foreground;
}

/*******************************************
bgSubstractor : source Image (binary Image) ,
destination Image, dilate & Erode matrix , iteration time
= let forgroung connect
+ dilate
+ erode
********************************************/
void bgSubMog::dilateErode(Mat* srcImg,Mat* dstImg,Mat element,int iteration){
     this->myLog->printLog(logAddMem("bgSubMog::dilateErode() enter"));
    for(int i= 0; i < iteration ; i++){
        dilate(*srcImg, *dstImg,element);
        erode(*dstImg, *dstImg,element);
    }
    this->myLog->printLog(logAddMem("bgSubMog::dilateErode() exit"));
}


/*******************************************
connectComponent : foreground Image(binary),
minimun object size to filter
= Get a Rectangle list for connect component
+ findContours
+ connect each Contours
********************************************/
vector<Rect> bgSubMog::connectComponent(const Mat mask, int min_obj_size)
{
    this->myLog->printLog(logAddMem("bgSubMog::connectComponent() enter"));
    Mat temp;
    mask.copyTo(temp);
    vector<Rect> Rects;
    int min_obj_height = mask.rows / 3 ;

    vector< vector<Point> > contours;
    findContours(temp, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    vector< vector<Point> >::const_iterator itContours = contours.begin();
    for(;itContours != contours.end();++itContours){
        Rect r = boundingRect(*itContours);
        if (r.width*r.height > min_obj_size && r.height > min_obj_height){
            Rects.push_back(r);
        }
    }
    this->myLog->printLog(logAddMem("bgSubMog::connectComponent() exit"));
    return Rects;
}



/*******************************************
objTrack :
= return forground mask
********************************************/
Mat bgSubMog::getMask(){
    this->myLog->printLog(logAddMem("bgSubMog::getMask()"));
    return forgroundMask;
}

/*******************************************
objTrack :
= return number of mask pixel
********************************************/
vector<Rect> bgSubMog::objTrack(const Mat frame,int min_obj_size)
{
    this->myLog->printLog(logAddMem("bgSubMog::objTrack() enter"));
    forgroundMask = bgSubtractor(frame);
    vector<Rect> rects = connectComponent(forgroundMask,min_obj_size);
    this->myLog->printLog(logAddMem("bgSubMog::objTrack() exit"));
    return rects;
}

string bgSubMog::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}


