#ifndef __HEADER_H__
#define __HEADER_H__

#include "stdafx.h"
#include "MyClass.h"
#include "outputLog.h"

using namespace std;
using namespace cv;

#define  KeyBoard_SPACE         32          //空白鍵
#define  KeyBoard_ESC           27          //ESC鍵

//是否要顯示Debug〔影像〕
//#define __debugShow__
#ifdef __debugShow__
    #define	 debugShowImage(windName,img)		cvShowImage(windName,img)
#else
    #define	 debugShowImage(windName,img)		;
#endif

//是否要印出Debug〔訊息〕
#ifdef __debugPrintf__
    #define	 debugPrintf(format,...)		printf(format,...)
#else
    #define	 debugPrintf(format,...)		;
#endif

/************************************************************************/
/*〔全域變數〕
/************************************************************************/
extern CvMemStorage* storage;
/******************************************************************************
*								〔檔頭定義區塊〕
*******************************************************************************
/************************************************
*〔imageProcess.cpp〕基本影像處理程序檔頭
************************************************/
void clearImgNoise(IplImage *srcImg,IplImage *destImg);
void colorAND(IplImage *srcImg,IplImage *mask,IplImage *destImg);
void drawRectangle(IplImage *destImg,CvRect rect,CvScalar color,int thickness = 1,int type = 8);
double getAngle(CvPoint &pointO, CvPoint& pointA, CvPoint& pointB);
double cos2angle(const double &cosValue);
double determinant_3x3(double x1, double y1, double x2, double y2, double x3, double y3);
double crossProduct(CvPoint pointO, CvPoint pointA, CvPoint pointB);
void bgThreshold(IplImage *srcImg,IplImage *destImg,Scalar bgColor);

/************************************************
*〔type.cpp〕型態轉換檔頭
************************************************/
cv::Mat Ipl2Mat(IplImage *img);
IplImage* Mat2Ipl(cv::Mat mat);
IplImage* getGrayImg(IplImage* srcImg);
char *getFileName(char *filePath);
bool getCenter(CvArr *src, CvPoint &center);
CvPoint2D32f Point2Point32f(CvPoint point);
cv::Scalar getRandColor();
double getSlope(CvPoint startPoint,CvPoint endPoint);
double getDistance(CvPoint point1,CvPoint point2);
CvPoint getMedian(CvPoint point1,CvPoint point2);
void CopyMat2Ipl(const Mat &srcImgMat,IplImage **srcImgIpl);

/************************************************
*〔cellDetect.cpp〕手部偵測檔頭
************************************************/
bool cellDetect(const Mat &srcImgMat);
//bool cellDetect(IplImage* srcImg);
bool findObject(IplImage* srcImg,IplImage *skinGrayImg, CvMemStorage* storage);
void find_2MaxArea(vector <HandArea*> &handArea,CvSeq *tour1,CvSeq *tour2);
bool checkHandWrap(IplImage *hankSkinImg);
void grabCutProcess(IplImage *srcImg,IplImage **destImg,IplImage *maskImg,CvRect ROI);
void getGrabCutMask(IplImage *srcImg,IplImage *maskImg,HandArea *hand);
bool cellAnalysis(IplImage *srcImg,IplImage *resultImg,IplImage *holdImg,IplImage *handMaskImg);

/************************************************
*〔skinDetecter.cpp〕連通元件檔頭
************************************************/
bool getSkinArea(IplImage *srcImg,IplImage **skinGrayImg);
void skinColorDetection(IplImage *srcImg,IplImage *destImg);
void connectTwoPassAlgo(const cv::Mat& _binImg, cv::Mat& _lableImg);
void connectShowColor(const cv::Mat& _labelImg, cv::Mat& _colorLabelImg);


#endif
