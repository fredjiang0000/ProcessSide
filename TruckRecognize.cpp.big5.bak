

#include "TruckRecognize.h"

#include <opencv2\core\core.hpp>

#include <opencv2\highgui\highgui.hpp>

#include <opencv2\imgproc\imgproc.hpp>

#include <iostream>

#include <fstream>

#include <list>

#include <opencv\cv.h>

#include <opencv\highgui.h>

#include <QtDebug>


using namespace cv;

using namespace std;


TruckRecognize::TruckRecognize(IplImage* img)
{
    originalImage = img;
    changeImage = NULL;
    drawImage = NULL;
    detectRegion = (img->width * img->height);
    storage = cvCreateMemStorage(0);
    allSquares = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), storage);

//    //read file to open append word
//    char * filename = "filename2.txt";
//    fstream fn;
//    fn.open(filename,ios::in);
//    if (!fn){
//        cout << "Fail to open file : " << filename << endl;
//    }
//    else{
//        char wordFN[30];
//        while (fn.getline(wordFN, sizeof(wordFN))){
//            IplImage *apWord = cvLoadImage(wordFN, CV_LOAD_IMAGE_UNCHANGED);
//            if (!apWord){
//                cout << "Error: Couldn't open the image file : " << filename << endl;
//            }
//            else{
//                appendWord.push_back(apWord);
//            }

//        }
//        fn.close();
//    }
}


TruckRecognize::TruckRecognize(IplImage* img, int detectR)
{
    originalImage = img;
    changeImage = NULL;
    drawImage = NULL;
    detectRegion = detectR;
    storage = cvCreateMemStorage(0);
    allSquares = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), storage);
}

TruckRecognize::~TruckRecognize()
{
    if (changeImage != NULL){
        cvReleaseImage(&changeImage);
        cout << "cvReleaseImage(&changeImage)" << endl;
    }
    if (drawImage != NULL){
        cvReleaseImage(&drawImage);
        cout << "cvReleaseImage(&drawImage)" << endl;
    }
//    if(originalImage != NULL){
//        cvReleaseImage(&originalImage);
//        cout << "cvReleaseImage(&originalImage)" << endl;
//    }
    cvReleaseMemStorage(&storage);
}

bool TruckRecognize::isTruck(){
    if (allSquares->total == 0){
        findSquares4(originalImage, storage, allSquares, detectRegion);
    }

    if (allSquares->total > 0)
    {
        drawSquares(originalImage, allSquares);
        return true;
    }
    else{
        return false;
    }
}

void TruckRecognize::findSquares4(IplImage* img, CvMemStorage* storage, CvSeq* squares, int detectRegion)

{

    CvSeq* contours;

    int i, c, l, N = 7;

    CvSize sz = cvSize(img->width & -2, img->height & -2);

    IplImage* timg = cvCloneImage(img); // make a copy of input image

    IplImage* gray = cvCreateImage(sz, 8, 1);

    IplImage* pyr = cvCreateImage(cvSize(sz.width / 2, sz.height / 2), 8, 3);

    IplImage* tgray;

    CvSeq* result;

    double s, t;
    int recThresholds = detectRegion / 20;
    int recThresholdb = detectRegion / 2;

    // create empty sequence that will contain points -

    // 4 points per square (the square's vertices)
    //squares = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), storage);
    //---MOVE to initial


    // select the maximum ROI in the image

    // with the width and height divisible by 2

    cvSetImageROI(timg, cvRect(0, 0, sz.width, sz.height));



    // down-scale and upscale the image to filter out the noise

    cvPyrDown(timg, pyr, 7);

    cvPyrUp(pyr, timg, 7);

    tgray = cvCreateImage(sz, 8, 1);



    // find squares in every color plane of the image

    for (c = 0; c < 3; c++)

    {

        // extract the c-th color plane

        cvSetImageCOI(timg, c + 1);

        cvCopy(timg, tgray, 0);



        // try several threshold levels

        for (l = 0; l < N; l++)

        {
            // hack: use Canny instead of zero threshold level.

            // Canny helps to catch squares with gradient shading

            if (l == 0)

            {

                // apply Canny. Take the upper threshold from slider

                // and set the lower to 0 (which forces edges merging)

                cvCanny(tgray, gray, 0, cannyThresh, 5);

                // dilate canny output to remove potential

                // holes between edge segments

                cvDilate(gray, gray, 0, 1);

                //cvNamedWindow("canny", 1);
                //cvShowImage("canny", gray);
                //c = cvWaitKey(0);

            }

            else

            {

                // apply threshold if l!=0:

                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0

                cvThreshold(tgray, gray, (l + 1) * 255 / N, 255, CV_THRESH_BINARY);
                //cvThreshold(tgray, gray, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                //if (l != N-1){

                //cvNamedWindow("binary", 1);
                //cvShowImage("binary", gray);
                //	c = cvWaitKey(0);

                //}
            }


            //cvNamedWindow("findcorner", 1);
            //cvShowImage("findcorner", gray);
            //c = cvWaitKey(0);

            // find contours and store them all as a list

            cvFindContours(gray, storage, &contours, sizeof(CvContour),

                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));



            // test each contour

            while (contours)

            {

                // approximate contour with accuracy proportional

                // to the contour perimeter

                result = cvApproxPoly(contours, sizeof(CvContour), storage,

                    CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);

                // square contours should have 4 vertices after approximation

                // relatively large area (to filter out noisy contours)

                // and be convex.

                // Note: absolute value of an area is used because

                // area may be positive or negative - in accordance with the

                // contour orientation

                if (result->total == 4 &&

                    fabs(cvContourArea(result, CV_WHOLE_SEQ)) > recThresholds &&

                    fabs(cvContourArea(result, CV_WHOLE_SEQ)) < recThresholdb &&

                    cvCheckContourConvexity(result))

                {

                    s = 0;



                    for (i = 0; i < 5; i++)

                    {

                        // find minimum angle between joint

                        // edges (maximum of cosine)

                        if (i >= 2)

                        {

                            t = fabs(angle(

                                (CvPoint*)cvGetSeqElem(result, i),

                                (CvPoint*)cvGetSeqElem(result, i - 2),

                                (CvPoint*)cvGetSeqElem(result, i - 1)));

                            s = s > t ? s : t;

                        }

                    }





                    // if cosines of all angles are small

                    // (all angles are ~90 degree) then write quandrange

                    // vertices to resultant sequence

                    if (s < 0.5)

                        for (i = 0; i < 4; i++)

                            cvSeqPush(squares,

                            (CvPoint*)cvGetSeqElem(result, i));

                }

                // take the next contour

                contours = contours->h_next;

            }

        }

    }




    // release all the temporary images

    cvReleaseImage(&gray);

    cvReleaseImage(&pyr);

    cvReleaseImage(&tgray);

    cvReleaseImage(&timg);

}

void TruckRecognize::drawSquares(IplImage* img, CvSeq* squares)

{
    const char* wndname = "Square Detection Draw";

    CvSeqReader reader;

    drawImage = cvCloneImage(img);

    CvPoint pt[4];

    int i;

    // initialize reader of the sequence

    cvStartReadSeq(squares, &reader, 0);

    // read 4 sequence elements at a time (all vertices of a square)

    for (i = 0; i < squares->total; i += 4)

    {

        CvPoint* rect = pt;

        int count = 4;



        // read 4 vertices

        memcpy(pt, reader.ptr, squares->elem_size);

        CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

        memcpy(pt + 1, reader.ptr, squares->elem_size);

        CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

        memcpy(pt + 2, reader.ptr, squares->elem_size);

        CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

        memcpy(pt + 3, reader.ptr, squares->elem_size);

        CV_NEXT_SEQ_ELEM(squares->elem_size, reader);



        // draw the square as a closed polyline
        cvPolyLine(drawImage, &rect, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0);
    //	cout << "+++++++++++++++++++++++++++++++++++++++" << endl;
        qDebug() << "draw the square as a closed polyline";
        cout << pt[0].x << " , " << pt[0].y << " : " << pt[1].x << " , " << pt[1].y << " : " << pt[2].x << " , " << pt[2].y << " : " << pt[3].x << " , " << pt[3].y << endl;


    }

    //cout << "==================================================================" << endl;

    // show the resultant image

    cvShowImage(wndname, drawImage);
    cvWaitKey(1);

}

double TruckRecognize::angle(CvPoint* pt1, CvPoint* pt2, CvPoint* pt0)

{

    double dx1 = pt1->x - pt0->x;

    double dy1 = pt1->y - pt0->y;

    double dx2 = pt2->x - pt0->x;

    double dy2 = pt2->y - pt0->y;

    return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);

}


//		[0]		[3]
//		[1]		[2]

IplImage* TruckRecognize::getEndoRect(IplImage* srcImg, CvSeq* squares){

        CvPoint pt[4];
        CvSeqReader reader;
        cvStartReadSeq(squares, &reader, 0);
        int i;
        int xBig, xSmall, yBig, ySmall;
        int x[4];
        int y[4];

        for (i = 0; i < squares->total; i += 4)
        {
            //CvPoint* rect = pt;
            int count = 4;

            // read 4 vertices

            memcpy(pt, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            memcpy(pt + 1, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            memcpy(pt + 2, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            memcpy(pt + 3, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            int j;
            for (j = 0; j < 4; j++){
                x[j] = pt[j].x;
                y[j] = pt[j].y;
            }

            std::sort(x, x + 4);
            std::sort(y, y + 4);
            if (i == 0){
                xSmall = x[1];
                xBig = x[2];
                ySmall = y[1];
                yBig = y[2];
            }
            else{
                if (x[1] < xSmall)
                    xSmall = x[1];
                if (x[2] > xBig)
                    xBig = x[2];
                if (y[1] < ySmall)
                    ySmall = y[1];
                if (y[2] > yBig)
                    yBig = y[2];
            }
        }

        CvPoint endoRec[4];
        endoRec[0].x = xSmall;
        endoRec[0].y = ySmall;
        endoRec[1].x = xBig;
        endoRec[1].y = ySmall;
        endoRec[2].x = xBig;
        endoRec[2].y = yBig;
        endoRec[3].x = xSmall;
        endoRec[3].y = yBig;

        CvSize size = cvSize(xBig - xSmall, yBig - ySmall);
        IplImage* dstImg = cvCreateImage(size, IPL_DEPTH_8U, 3);
        CvRect rect_roi = cvRect(xSmall, ySmall, xBig - xSmall, yBig - ySmall);
        dstImg = cvCloneImage(srcImg);

        cvSetImageROI(dstImg, rect_roi);
        return dstImg;
}

IplImage*  TruckRecognize::getExtendRect(IplImage* srcImg, CvSeq* squares){
        CvPoint pt[4];
        CvPoint2D32f srcRec[4];
        int recSize;
        CvSeqReader reader;
        cvStartReadSeq(squares, &reader, 0);
        int i;
        int xBig, xSmall, yBig, ySmall;
        int x[4];
        int y[4];
        for (i = 0; i < squares->total; i += 4)
        {
            CvPoint* rect = pt;
            int count = 4;

            // read 4 vertices

            memcpy(pt, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            memcpy(pt + 1, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            memcpy(pt + 2, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            memcpy(pt + 3, reader.ptr, squares->elem_size);

            CV_NEXT_SEQ_ELEM(squares->elem_size, reader);

            // find max size Rect and record setup

            int j;
            for (j = 0; j < 4; j++){
                x[j] = pt[j].x;
                y[j] = pt[j].y;
            }

            std::sort(x, x + 4);
            std::sort(y, y + 4);
            if (i == 0){
                xSmall = x[0];
                xBig = x[3];
                ySmall = y[0];
                yBig = y[3];
                recSize = (x[3] - x[0]) * (y[3] - y[0]);

                srcRec[0].x = pt[0].x;
                srcRec[0].y = pt[0].y;
                srcRec[2].x = pt[2].x;
                srcRec[2].y = pt[2].y;
                if (pt[1].x < pt[3].x){
                    srcRec[1].x = pt[1].x;
                    srcRec[1].y = pt[1].y;
                    srcRec[3].x = pt[3].x;
                    srcRec[3].y = pt[3].y;
                }
                else{
                    srcRec[1].x = pt[3].x;
                    srcRec[1].y = pt[3].y;
                    srcRec[3].x = pt[1].x;
                    srcRec[3].y = pt[1].y;
                }

            }
            else{
                if (x[0] < xSmall)
                    xSmall = x[0];
                if (x[3] > xBig)
                    xBig = x[3];
                if (y[0] < ySmall)
                    ySmall = y[0];
                if (y[3] > yBig)
                    yBig = y[3];
                if (((x[3] - x[0]) * (y[3] - y[0])) > recSize){
                    recSize = (x[3] - x[0]) * (y[3] - y[0]);
                    srcRec[0].x = pt[0].x;
                    srcRec[0].y = pt[0].y;
                    srcRec[2].x = pt[2].x;
                    srcRec[2].y = pt[2].y;
                    if (pt[1].x < pt[3].x){
                        srcRec[1].x = pt[1].x;
                        srcRec[1].y = pt[1].y;
                        srcRec[3].x = pt[3].x;
                        srcRec[3].y = pt[3].y;
                    }
                    else{
                        srcRec[1].x = pt[3].x;
                        srcRec[1].y = pt[3].y;
                        srcRec[3].x = pt[1].x;
                        srcRec[3].y = pt[1].y;
                    }
                }
            }
        } // end for all squares
        CvPoint2D32f dstchange[4];
        dstchange[0].x = 0;
        dstchange[0].y = 0;
        dstchange[1].x = 0;
        dstchange[1].y = yBig - ySmall + 1;
        dstchange[2].x = xBig - xSmall + 1;
        dstchange[2].y = yBig - ySmall + 1;
        dstchange[3].x = xBig - xSmall + 1;
        dstchange[3].y = 0;
        CvMat* warp_mat = cvCreateMat(3, 3, CV_32FC1);
        CvSize size = cvSize(xBig - xSmall, yBig - ySmall);
        IplImage* dstImg = cvCreateImage(size, IPL_DEPTH_8U, 3);
        cvGetPerspectiveTransform(srcRec, dstchange, warp_mat);
        cvWarpPerspective(srcImg, dstImg, warp_mat);

        return dstImg;

}

//Mat no relese (is Need ? )
bool TruckRecognize::isWhite(){
    if (allSquares->total > 0){
        /***** Get Endo Rectangle *****/
        if (changeImage != NULL){
            cvReleaseImage(&changeImage);
        }
        changeImage = getEndoRect(originalImage, allSquares);

        /***** Calculate Histagram for Endo Rectangle *****/
        /// Separate the image in 3 places ( B, G and R )
        vector<Mat> bgr_planes;
        split(changeImage, bgr_planes);

        /// Establish the number of bins
        int histSize = 256;

        /// Set the ranges ( for B,G,R) )
        float range[] = { 0, 256 };
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        Mat b_hist, g_hist, r_hist;

        /// Compute the histograms:
        calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
        calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
        calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

        int allR = 0, allG = 0, allB = 0, allhist = 0, i;
        for (i = 0; i < (int)range[1]; i++)
        {
            allhist += ((int)r_hist.at <float>(i) +(int)g_hist.at <float>(i) +(int)b_hist.at <float>(i));
            if (i > 150){
                allR += (int)r_hist.at <float>(i);
                allG += (int)g_hist.at <float>(i);
                allB += (int)b_hist.at <float>(i);
            }
        }

        /***** Deside Rectangle is near White *****/
        float colorThr = 0.7;
        float lightRatio = (allR + allG + allB) / (float)allhist;
        float colorRatio[3] = { allR / (float)allG, allR / (float)allB, allG / (float)allB };
        std::sort(colorRatio, colorRatio + 3);
        bool isGray = !(colorRatio[0] < 0.9 || colorRatio[2] > 1.1);

        if (lightRatio > colorThr && isGray){
            cout << "This car is white." << endl;
            return true;
        }

    }
    return false;
}

bool TruckRecognize::isCompanyCar()
{
    try
    {
        if (this->isTruck()){
            qDebug() << "this->isTruck()";
            if (this->isWhite()){
                qDebug() << "this->isWhite()";
                if (this->isAppoint()){
//                    system("CLS");
                    qDebug() << "判定為公司車";
                    return true;
                }
                else{
//                    system("CLS");
                    qDebug() << "判定為非公司的白色卡車";
                    return false;
                }
                return true;
            }
            else{
//                system("CLS");
                qDebug() << "判定為非公司的非白色卡車";
                return false;
            }
        }
        else {
//            system("CLS");
            qDebug() << "判定為非卡車" ;
            return false;
        }
    }
    catch( cv::Exception& e )
    {
        const char* err_msg = e.what();
        std::cout << "exception caught: " << err_msg << std::endl;
    }
}


bool TruckRecognize::isAppoint(){
//    const char* wndname = "Appoint";
    if (allSquares->total > 0){
        /***** Get Extend Rectangle *****/
        if (changeImage != NULL){
            qDebug() << "Stage1";
            cvReleaseImage(&changeImage);
            changeImage = NULL;
            qDebug() << "Stage2";
        }
        qDebug() << "Stage3";
        changeImage = getExtendRect(originalImage, allSquares);
        qDebug() << "Stage4";
        /***** Find Text in  Extend Rectangle *****/
        qDebug() << "Stage5";
        CvSize sz = cvSize(changeImage->width, changeImage->height);
        qDebug() << "Stage6";
        IplImage* gray = cvCreateImage(sz, 8, 1);
        qDebug() << "Stage7";
        cvCvtColor(changeImage, gray, CV_BGR2GRAY);
        qDebug() << "Stage8";
        IplImage* otsu = cvCreateImage(sz, 8, 1);
        qDebug() << "Stage9";
        cvCopy(gray, otsu, 0);
        qDebug() << "Stage10";
        cvThreshold(gray, otsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
        qDebug() << "Stage11";

        //Find text region START===>>>>>
        int textThres = 5;
        Mat changeMat = Mat(otsu, 0);
        int x, y;
        int state = 0; // 1=text start ; 0=text end
        //Find text region for ROW
        int rowPixel, lastRow, nowRow;
        list<Point> ylist;
        for (y = 0; y < changeMat.rows; y++){
            rowPixel = 0;
            for (x = 10; x < changeMat.cols; x++){
                Scalar rgb = changeMat.at<uchar>(y, x);
                if ((int)rgb[0] < 220){
                    rowPixel++;
                }
            }

            if (y == 0){
                lastRow = rowPixel;
            }
            else
            {
                lastRow = nowRow;
            }
            nowRow = rowPixel;

            if (((nowRow ) > 0) && state == 0){
                //text start
                state = 1;
                Point textRegion;  //start x,y end x,y
                textRegion.x = y;
                ylist.push_back(textRegion);
            }
            if (((nowRow == 0) && lastRow == 0) && state == 1){
                //text end
                state = 0;
                Point temp;
                temp = ylist.back();
                temp.y = y;
                ylist.pop_back();
                ylist.push_back(temp);
            }
        }
        qDebug() << "Stage12";
        IplImage* textRegion = cvCreateImage(sz, IPL_DEPTH_8U, 3);
        cvCopy(changeImage, textRegion, 0);
        int count = 4;

        //Find text region for COLUME
        list<CvPoint* > textlist;
        int startY, endY;
        int colPixel, lastCol, nowCol;
        state = 0;
        CvPoint *tempTextRegion1 = NULL;
        CvPoint * tempTextRegion2 = NULL;
        while (!ylist.empty() ){
            startY = ylist.front().x;
            endY = ylist.front().y;
            ylist.pop_front();
            textThres = (endY - startY)/4;
            for (x = 0; x < changeMat.cols; x++){
                colPixel = 0;
                for (y = startY; y < endY; y++){
                    Scalar rgb = changeMat.at<uchar>(y, x);
                    if ((int)rgb[0] < 220){
                        colPixel++;
                    }
                }

                if (x == 0){
                    lastCol = colPixel;
                }
                else
                {
                    lastCol = nowCol;
                }
                nowCol = colPixel;

                if (((nowCol ) > 0) && state == 0){
                    //text start
                    state = 1;
                    tempTextRegion1 = new CvPoint[4];
                    tempTextRegion1[0].x = x;
                    tempTextRegion1[0].y = startY;
                    tempTextRegion1[1].x = x;
                    tempTextRegion1[1].y = endY;
                    tempTextRegion1[2].y = endY;
                    tempTextRegion1[3].y = startY;                   
                    textlist.push_back(tempTextRegion1);                 
                    qDebug() << "tempTextRegion1:(" << tempTextRegion1[0].x << "," << tempTextRegion1[0].y << ")";
                }
                if (((nowCol == 0) && lastCol == 0) && state == 1){
                    //text end
                    state = 0;
                    tempTextRegion2 = new CvPoint[4];
                    tempTextRegion2[0].x = textlist.back()[0].x;
                    tempTextRegion2[0].y = textlist.back()[0].y;
                    tempTextRegion2[1].x = textlist.back()[1].x;
                    tempTextRegion2[1].y = textlist.back()[1].y;
                    tempTextRegion2[2].x = x;
                    tempTextRegion2[2].y = textlist.back()[2].y;
                    tempTextRegion2[3].x = x;
                    tempTextRegion2[3].y = textlist.back()[3].y;

                    //draw squares
                    cvPolyLine(textRegion, &tempTextRegion2, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0);

                    delete *textlist.end();
                    textlist.pop_back();
                    textlist.push_back(tempTextRegion2);
                    qDebug() << "tempTextRegion2:(" << tempTextRegion2[0].x << "," << tempTextRegion2[0].y << ")";
                }
            }//end of for
            if (state == 1)
            {
                state = 0;
                CvPoint *tempTextRegion = new CvPoint[4];
                tempTextRegion[0].x = textlist.back()[0].x;
                tempTextRegion[0].y = textlist.back()[0].y;
                tempTextRegion[1].x = textlist.back()[1].x;
                tempTextRegion[1].y = textlist.back()[1].y;
                tempTextRegion[2].x = x;
                tempTextRegion[2].y = textlist.back()[2].y;
                tempTextRegion[3].x = x;
                tempTextRegion[3].y = textlist.back()[3].y;

                delete *textlist.end();
                textlist.pop_back();
                textlist.push_back(tempTextRegion);
            }
            else{
                CvPoint* textRegion;
                textRegion = textlist.back();
            }
        } //while yList not empty
        qDebug() << "Stage13";
        // <<<<<===== Find text region END


        //cvShowImage(wndname, textRegion);

        /***** Do Normalization for each Letter *****/
         qDebug() << "Stage14";
        wordNormailization(textlist, otsu);
         qDebug() << "Stage15";

        /***** Open Letter template *****/
        //read file to open append word
        list<IplImage * > appendWord;
        char * filename = "filename2.txt";
        fstream fn;
        fn.open(filename,ios::in);
        if (!fn){//狦秨币郎ア毖fp0Θfp獶0
            cout << "Fail to open file : " << filename << endl;
        }
        else{
            char wordFN[30];
             IplImage *apWord = NULL;
            while (fn.getline(wordFN, sizeof(wordFN))){
                apWord = cvLoadImage(wordFN, CV_LOAD_IMAGE_UNCHANGED);
                if (!apWord){
                    cout << "Error: Couldn't open the image file : " << filename << endl;
                }
                else{
                    appendWord.push_back(apWord);
                }

            }
            fn.close();
        }

        /***** Find best Similar and deside Matching *****/
        if (wordSimilar(textlist, appendWord, gray)){
            qDebug() << "Stage16";
            // Release Image & list
            cvReleaseImage(&gray);
            cvReleaseImage(&otsu);
            cvReleaseImage(&textRegion);
            qDebug() << "Stage17";
            cout << "textlist.size()1:" << textlist.size() << endl;
            while (textlist.size() != 0)
                {
                    delete *textlist.begin();

                    textlist.pop_front();
                }
            cout << "textlist.size()2:" << textlist.size() << endl;
            qDebug() << "Stage18";
            cout << "appendWord.size()1:" << appendWord.size() << endl;
            while (!appendWord.empty()){
                IplImage * temp = appendWord.front();
                appendWord.pop_front();
                cvReleaseImage(&temp);
            }
            cout << "appendWord.size()2:" << appendWord.size() << endl;
            return true;
        }
        else{
            qDebug() << "Stage19";
            // Release Image & list
            cvReleaseImage(&gray);
            cvReleaseImage(&otsu);
            cvReleaseImage(&textRegion);
            qDebug() << "Stage20";
            cout << "textlist.size()3:" << textlist.size() << endl;
            while (textlist.size() != 0)
                {
                    delete *textlist.begin();

                    textlist.pop_front();
                }
            cout << "textlist.size()4:" << textlist.size() << endl;
            qDebug() << "Stage18";
            cout << "appendWord.size()3:" << appendWord.size() << endl;
            while (!appendWord.empty()){
                IplImage * temp = appendWord.front();
                appendWord.pop_front();
                cvReleaseImage(&temp);
            }
            cout << "appendWord.size()4:" << appendWord.size() << endl;
            qDebug() << "Stage21";
            return false;
        }
        qDebug() << "Stage22";

    } // end if squares > 0
    else
        return false;
}

void TruckRecognize::wordNormailization(list<CvPoint* > wordList, IplImage* img){

    qDebug() << "wordNormailization 1";
    // input img need otsu
    int i=0,x,y;
    IplImage* tWord = NULL;
    int tPixel;
    int rgbThresh = 250;
//	const char* wndname = "_545145154745515794576576264656426121675467546756476567467456746451";
//	const char* wndname1 = "_wordNormailization1234567890asdzxcqwertyfghvbnuiojklmzxcasdweersdf";
    list<CvPoint* > normalList;
    int listSize = wordList.size();
    qDebug() << "wordNormailization 2";
    while (!wordList.empty()){
        qDebug() << "wordNormailization 3";
        i++;
        CvPoint* tWordRec = new CvPoint[4];
        tWordRec = wordList.front();
        tWord = cvCloneImage(img);
        CvRect rect_roi = cvRect(tWordRec[0].x, tWordRec[0].y, tWordRec[2].x - tWordRec[0].x, tWordRec[2].y - tWordRec[0].y);
        qDebug() << "tWord->width:" << tWord->width << " tWord->height:" << tWord->height;
        qDebug() << "rect_roi.x:" << rect_roi.x << " rect_roi.y:" << rect_roi.y << " rect_roi.width:" << rect_roi.width << " rect_roi.height:" << rect_roi.height;
        cvSetImageROI(tWord, rect_roi);
//		cvShowImage(wndname + i, tWord);
        Mat tWordMat = Mat(tWord, 0);
        qDebug() << "wordNormailization 4";
            //for top
            for (y = 0; y < tWordMat.rows; y++){
                tPixel = 0;
                for (x = 0; x < tWordMat.cols; x++)
                {
                    Scalar rgb = tWordMat.at<uchar>(y, x);
                    if ((int)rgb[0] < rgbThresh){
                        tPixel++;
                    }
                }
                if (tPixel != 0)
                {
                    break;
                }
            }
            qDebug() << "wordNormailization 5";
            tWordRec[0].y += y;
            tWordRec[3].y += y;

            //for down
            for (y = tWordMat.rows - 1; y >= 0; y--){
                tPixel = 0;
                for (x = 0; x < tWordMat.cols; x++)
                {
                    Scalar rgb = tWordMat.at<uchar>(y, x);
                    if ((int)rgb[0] < rgbThresh){
                        tPixel++;
                    }
                }
                if (tPixel != 0)
                {
                    break;
                }
            }
            qDebug() << "wordNormailization 6";
            tWordRec[1].y = tWordRec[1].y - ((tWordMat.rows - 1) - y);
            tWordRec[2].y = tWordRec[2].y - ((tWordMat.rows - 1) - y);

            //for left
            for (x = 0; x < tWordMat.cols; x++){
                tPixel = 0;
                for (y = 0; y < tWordMat.rows; y++)
                {
                    Scalar rgb = tWordMat.at<uchar>(y, x);
                    if ((int)rgb[0] < rgbThresh){
                        tPixel++;
                    }
                }
                if (tPixel != 0)
                {
                    break;
                }
            }
            qDebug() << "wordNormailization 7";
            tWordRec[0].x += x;
            tWordRec[1].x += x;

            //for right
            for (x = tWordMat.cols - 1; x >= 0; x--){
                tPixel = 0;
                for (y = 0; y < tWordMat.rows; y++)
                {
                    Scalar rgb = tWordMat.at<uchar>(y, x);
                    if ((int)rgb[0] < rgbThresh){
                        tPixel++;
                    }
                }
                if (tPixel != 0)
                {
                    break;
                }
            }
            qDebug() << "wordNormailization 8";
            tWordRec[3].x = tWordRec[3].x - ((tWordMat.cols - 1) - x);
            tWordRec[2].x = tWordRec[2].x - ((tWordMat.cols - 1) - x);

//            delete *wordList.begin();
            wordList.pop_front();
            qDebug() << "wordNormailization 9";
            normalList.push_back(tWordRec);
            qDebug() << "wordNormailization 10";
        }

        //Release wordList
    qDebug() << "wordNormailization 11";
    qDebug() << "wordList.size()1:" << wordList.size();
        while (!wordList.empty()){
             delete *wordList.begin();
            wordList.pop_front();
            qDebug() << "wordNormailization 12";
        }
        qDebug() << "wordList.size()2:" << wordList.size();
        qDebug() << "wordNormailization 13";
        wordList = normalList;
        qDebug() << "wordNormailization 14";

//        qDebug() << "normalList.size()1:" << normalList.size();
////        while (!normalList.empty()){
////             delete *normalList.begin();
////            normalList.pop_front();
////        }
//        qDebug() << "normalList.size()2:" << normalList.size();
        qDebug() << "wordNormailization 15";
        cvReleaseImage(&tWord);
        qDebug() << "wordNormailization 16";
}

struct matchStr{
    float crectRatio;
    CvPoint* coordinate;
    IplImage* templateImg;
};

bool TruckRecognize::wordSimilar(list<CvPoint*> wordList, list<IplImage*> apImgList, IplImage* img){

    qDebug() << "wordSimilar 1";
    const char* wndname = "__wordSimilarwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww";
    //const char* wndname1 = "__apImgListiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
    CvSize sz = cvSize(apImgList.front()->width, apImgList.front()->height);
    IplImage* tApImg = NULL;
    IplImage* tWordImg = cvCreateImage(sz, 8, 1);
    IplImage* drawImg = cvCloneImage(img);
    qDebug() << "wordSimilar 2";
    list<matchStr> matchList;
    IplImage* tWord = NULL;
    qDebug() << "wordList.size()0:" << wordList.size();
    //reSize wordList
    for (int i = wordList.size(); i > 0; i--){
        CvPoint* tWordRec = new CvPoint[4];
        qDebug() << "wordList.size()1:" << wordList.size();
        tWordRec = wordList.front();
        qDebug() << "wordList.size()2:" << wordList.size();
        wordList.pop_front();
        qDebug() << "wordList.size()3:" << wordList.size();
        wordList.push_back(tWordRec);
        qDebug() << "wordList.size()4:" << wordList.size();
        qDebug() << "wordSimilar 3";
        tWord = cvCloneImage(img);
        qDebug() << "wordSimilar 3-1";
        qDebug() << "tWordRec[2].x:" << tWordRec[2].x << " tWordRec[0].x:" << tWordRec[0].x;
        qDebug() << "tWordRec[2].y:" << tWordRec[2].y << " tWordRec[0].y:" << tWordRec[0].y;
        CvRect rect_roi = cvRect(tWordRec[0].x, tWordRec[0].y, tWordRec[2].x - tWordRec[0].x, tWordRec[2].y - tWordRec[0].y);
        qDebug() << "wordSimilar 3-2";
        qDebug() << "tWord->width:" << tWord->width << " tWord->height:" << tWord->height;
        qDebug() << "rect_roi.x:" << rect_roi.x << " rect_roi.y:" << rect_roi.y << " rect_roi.width:" << rect_roi.width << " rect_roi.height:" << rect_roi.height;
        cvSetImageROI(tWord, rect_roi);
        qDebug() << "wordSimilar 4";
        cvResize(tWord, tWordImg);
        cvThreshold(tWordImg, tWordImg, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
        Mat tWordMat(tWordImg);
//		imshow(wndname + i, tWordMat);
        qDebug() << "wordSimilar 5";
        int x, y;
        for (int j = apImgList.size(); j > 0; j--){
            tApImg = apImgList.front();
            Mat tApImgMat(tApImg);
//            delete *apImgList.begin();
            apImgList.pop_front();
            apImgList.push_back(tApImg);
            qDebug() << "wordSimilar 6";
            // get error pixel
            int crrPix = 0;
            for (y = 0; y < tWordMat.rows; y++){
                for (x = 10; x < tWordMat.cols; x++){
                    Scalar apImgRGB = tApImgMat.at<uchar>(y, x);
                    Scalar wordRGB = tWordMat.at<uchar>(y, x);
                    if ((int)apImgRGB[0] == (int)wordRGB[0]){
                        crrPix++;
                    }
                }
            }
            qDebug() << "wordSimilar 7";
//			cout << "Template: " << j << " correct Pixe = " << crrPix / (float)(tWordMat.rows * tWordMat.cols) << endl;
            //insert & update to matchList
            matchStr tData;
            tData.crectRatio = crrPix / (float)(tWordMat.rows * tWordMat.cols);
            tData.coordinate = tWordRec;
            tData.templateImg = tWord;
            if (matchList.size() < apImgList.size()){
                matchList.push_back(tData);
            }
            else{
                matchStr oriData;
                oriData = matchList.front();
                matchList.pop_front();
                if (tData.crectRatio > oriData.crectRatio){
                    matchList.push_back(tData);
                }
                else
                {
                    matchList.push_back(oriData);
                }
            }
            qDebug() << "wordSimilar 8";
        }
        cvReleaseImage(&tWord);
    }
    qDebug() << "wordSimilar 9";

    cvReleaseImage(&tApImg);
    qDebug() << "wordSimilar 10";
    cvReleaseImage(&tWordImg);
    qDebug() << "wordSimilar 11";

    //Show Result and analysis
    int count = 4;
    list<int> xlist, ylist;
    int width = 0;
    for (int i = matchList.size(); i > 0; i--){
        matchStr oriData;
        oriData = matchList.front();
        matchList.pop_front();
        matchList.push_back(oriData);
//		cout << " Match Ratio: " << oriData.crectRatio << " ; Coordinate : "
//			<< oriData.coordinate[0].x << " , " << oriData.coordinate[0].y  << " | "
//			<< oriData.coordinate[1].x << " , " << oriData.coordinate[1].y << " | "
//			<< oriData.coordinate[2].x << " , " << oriData.coordinate[2].y << " | "
//			<< oriData.coordinate[3].x << " , " << oriData.coordinate[3].y << " | " << endl;
        cvPolyLine(drawImg, &oriData.coordinate, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0);
//		cvShowImage(wndname + i, oriData.templateImg);
        cvShowImage("Match", drawImg);
        cvWaitKey(1);

        xlist.push_back(oriData.coordinate[0].x);
        ylist.push_back(oriData.coordinate[0].y);

        width += (oriData.coordinate[2].x - oriData.coordinate[0].x);
    }
    xlist.sort();
    ylist.sort();
    int nearThresh = 20;
    bool xNear = ((xlist.back() - xlist.front()) - width ) < nearThresh;
    bool yNear = (ylist.back() - ylist.front()) < nearThresh;
    qDebug() << "wordSimilar 12";

    qDebug() << "wordSimilar 13";
    if (xNear && yNear){
        cout << " Word Matching  " << endl;
        cvShowImage("Word Matching", drawImg);
        cvWaitKey(1);
        cvReleaseImage(&drawImg);
        return true;
    }
    cvReleaseImage(&drawImg);
    return false;
}

