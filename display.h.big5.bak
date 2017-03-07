#ifndef DISPLAY_H
#define DISPLAY_H

#include <iostream>

#include <opencv2/opencv.hpp>

#include <vector>

using namespace std;

using namespace cv;

class display
{
public:
    display();
    ~display();
    static void drawRects(Mat drawImg, vector<Rect> rectList);
    static void drawRectsColor(Mat drawImg, vector<Rect> rectList, Scalar color);
    static void drawRectsDifColor(Mat drawImg, vector<Rect> rectList);
    static void drawRect(Mat drawImg,Rect tempRect);

};

#endif // DISPLAY_H
