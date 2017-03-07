#ifndef __carRecognize_h_included__
#define __carRecognize_h_included__

#include <opencv2\core\core.hpp>

#include <opencv2\highgui\highgui.hpp>

#include <opencv2\imgproc\imgproc.hpp>

#include <opencv\cv.h>

#include <opencv\highgui.h>

#include <list>

using namespace cv;

using namespace std;

class TruckRecognize
{
    int detectRegion;
    IplImage* originalImage = NULL;
    IplImage* changeImage = NULL;
    IplImage* drawImage = NULL;
    int cannyThresh = 630;
    CvMemStorage* storage = 0;
    double angle(CvPoint* pt1, CvPoint* pt2, CvPoint* pt0);
    void wordNormailization( list<CvPoint*> wordList, IplImage* img);
    bool wordSimilar(list<CvPoint*> wordList, list<IplImage*> apImgList, IplImage* img);
    void findSquares4(IplImage* img, CvMemStorage* storage, CvSeq* squares, int detectRegion);
    IplImage* getEndoRect(IplImage* srcImg, CvSeq* squares);
    IplImage* getExtendRect(IplImage* srcImg, CvSeq* squares);
    void drawSquares(IplImage* img, CvSeq* squares);

public:
    CvSeq* allSquares = NULL;
    TruckRecognize(IplImage* img);
    TruckRecognize(IplImage* img, int detectR);
    ~TruckRecognize();
    bool isTruck();
    bool isAppoint();
    bool isWhite();
    bool isCompanyCar();
//private:
//    //read file to open append word
//    list<IplImage * > appendWord;
};


#endif

