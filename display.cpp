#include "display.h"

#include <iostream>

#include <opencv2/opencv.hpp>

#include <vector>

using namespace std;

using namespace cv;

display::display()
{

}
display::~display()
{

}

void display::drawRects(Mat drawImg,vector<Rect> rectList)
{
    if(rectList.size() > 0){
    for(int i = 0 ; i < rectList.size() ; ++i){
        rectangle(drawImg,Point( rectList[i].x , rectList[i].y ),
                  Point( rectList[i].x + rectList[i].width, rectList[i].y + rectList[i].height),
                  Scalar(0,0,255),3);
    }
    }
    return;
}

void display::drawRectsColor(Mat drawImg,vector<Rect> rectList,Scalar color)
{
    if(rectList.size() > 0){
    for(int i = 0 ; i < rectList.size() ; ++i){
        rectangle(drawImg,Point( rectList[i].x , rectList[i].y ),
                  Point( rectList[i].x + rectList[i].width, rectList[i].y + rectList[i].height),
                  color,3);
    }
    }
    return;
}

void display::drawRectsDifColor(Mat drawImg,vector<Rect> rectList)
{
    if(rectList.size() > 0){
    for(int i = 0 ; i < rectList.size() ; ++i){
        rectangle(drawImg,Point( rectList[i].x , rectList[i].y ),
                  Point( rectList[i].x + rectList[i].width, rectList[i].y + rectList[i].height),
                  Scalar(255,0,0),3);
    }
    }
    return;
}

void display::drawRect(Mat drawImg,Rect tempRect)
{
    rectangle(drawImg,Point( tempRect.x , tempRect.y ),
              Point(tempRect.x + tempRect.width, tempRect.y + tempRect.height),
              Scalar(255,0,0),3);

    return;
}
