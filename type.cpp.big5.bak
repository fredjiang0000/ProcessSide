#include "stdafx.h"
#include "header.h"
#include "outputLog.h"
using namespace std;

outputLog typeLog("type.log");
/************************************************************************/
/*[Function] 型態轉換::OpenCV IplImage => OpenCV Mat                    */
/************************************************************************/
cv::Mat Ipl2Mat(IplImage *img){
    typeLog.printLog("check thread Ipl2Mat()");
    return cv::Mat(img, 0);
}

///************************************************************************/
///*[Function] 型態轉換::OpenCV Mat => OpenCV IplImage                    */
///************************************************************************/
//IplImage* Mat2Ipl(cv::Mat mat){
//    return &IplImage(mat);
//}

/************************************************************************/
/*[Function] 型態轉換::CvPoint	=>	CvPoint2D32f
/************************************************************************/
CvPoint2D32f Point2Point32f(CvPoint point){
    typeLog.printLog("check thread Point2Point32f()");
    return cvPoint2D32f(point.x,point.y);
}

/************************************************************************/
/*[Function] 型態轉換::取得輸入影像的〔灰階影像〕
/************************************************************************/
IplImage* getGrayImg(IplImage* srcImg){
    typeLog.printLog("check thread getGrayImg() enter");
    //建立新影像
    IplImage *destImg = cvCreateImage(cvGetSize(srcImg), IPL_DEPTH_8U, 1);

    //灰階轉換
    cvCvtColor(srcImg,destImg,CV_BGR2GRAY);
    cvReleaseImage(&srcImg);
    typeLog.printLog("check thread getGrayImg() exit");
    return destImg;
}

/************************************************************************/
/*[Function] 取得『檔案路徑』上的〔檔案名稱〕(不含副檔名)
/************************************************************************/
char *getFileName(char *filePath){
    typeLog.printLog("check thread getFileName() enter");
    vector<char*> tokens;					//存放Token 暫存區
    char copyName[100];						//函式內要處理的路徑
    char *tokenPtr;							//Token指標

    //先將要處理的路徑做複製
    strcpy(copyName,filePath);


    //先放入第一個切割出來的Token
    tokenPtr = strtok(copyName,".\\");		//以「.」、「\」作為分隔符號
    tokens.push_back(tokenPtr);

    /*************************************
    *	之後便式逐一切割Token,直到Null
    **************************************/
    while(tokenPtr){

        if( (tokenPtr = strtok(NULL,".\\")) == NULL)		break;
        else												tokens.push_back(tokenPtr);
    }

    //最後再建立一個存放檔案名稱的容器
    char *fileName = (char*) malloc(sizeof(char) * strlen(tokens[tokens.size()-2]) );

    //將倒數第二個的Token複製到容器中	(PS:倒數第一個為附檔名)
    strcpy(fileName,tokens[tokens.size()-2]);
    typeLog.printLog("check thread getFileName() exit");
    return fileName;
}

/************************************************************************/
/*[Function] 取得"輪廓" 或 "影像" 中的〔重心〕位置
*〔參數〕
*	src		:	要檢查的〔輪廓〕或〔影像〕
*	center	:	要存放〔重心〕的容器
*
/************************************************************************/
bool getCenter(CvArr *src, CvPoint &center){
    typeLog.printLog("check thread getCenter() enter");

    if(src == NULL)
    {
        typeLog.printLog("check thread getCenter() exit1");
        return false;
    }

    double area, momentX, momentY;
    CvMoments moment;					//力矩(物理)

    //計算力矩
    cvMoments( src, &moment, 1);

    area = cvGetSpatialMoment( &moment, 0, 0 );		//求出面積

    if( area == 0){
        typeLog.printLog("check thread getCenter() exit2");
        return false;
    }

    momentX = cvGetSpatialMoment( &moment, 1, 0 );	//在各別求出Ｘ座標
    momentY = cvGetSpatialMoment( &moment, 0, 1 );	//與Ｙ座標

    center.x = (int) (momentX / area);
    center.y = (int) (momentY / area);
    typeLog.printLog("check thread getCenter() exit3");
    return true;
}

/************************************************************************/
/*[Function] 取得一個隨機的顏色 (彩色)
/************************************************************************/
cv::Scalar getRandColor() {
    typeLog.printLog("check thread getRandColor() enter");
    uchar r = 255 * (rand() / (1.0 + RAND_MAX) );
    uchar g = 255 * (rand() / (1.0 + RAND_MAX) );
    uchar b = 255 * (rand() / (1.0 + RAND_MAX) );
     typeLog.printLog("check thread getRandColor() exit");
    return cv::Scalar(b,g,r) ;
}

/************************************************************************/
/*[Function] 計算兩座標之間的〔斜率〕
/************************************************************************/
double getSlope(CvPoint startPoint,CvPoint endPoint){
    typeLog.printLog("check thread getSlope() enter");
    double delX = startPoint.x - endPoint.x;
    double delY = startPoint.y - endPoint.y;
    double slope = delY / (delX + 0.0001);
    typeLog.printLog("check thread getSlope() exit");
    return slope;
}

/************************************************************************/
/*[Function] 計算兩點之間的〔距離〕
/************************************************************************/
double getDistance(CvPoint point1,CvPoint point2){
    typeLog.printLog("check thread getDistance() enter");
    double dX = fabs( (double) point1.x - point2.x);
    double dY = fabs( (double) point1.y - point2.y);
    typeLog.printLog("check thread getDistance() exit");
    return sqrt( dX * dX + dY * dY );
}

/************************************************************************/
/*[Function] 計算兩點之間的〔中點〕
/************************************************************************/
CvPoint getMedian(CvPoint point1,CvPoint point2){
    typeLog.printLog("check thread getMedian()");
    return cvPoint( (point1.x + point2.x) >> 1 , (point1.y + point2.y) >> 1 );
}

/************************************************************************/
/*[Function] 將Ｍａｔ型態的格式 [完全複製]到 Ｉｐｌ格式的影像中
/************************************************************************/
void CopyMat2Ipl(const Mat &srcMatImg,IplImage **destIplImg){
    typeLog.printLog("check thread CopyMat2Ipl() enter");
    //先宣告Ipl影像型態的格式
    *destIplImg = cvCreateImage( cvSize(srcMatImg.cols,srcMatImg.rows), IPL_DEPTH_8U	,3	);

    //之後再去複製影像的內容
    for( int i = 0; i < srcMatImg.rows; i++ ) {
        for( int j = 0; j < srcMatImg.cols; j++ ) {
            cv::Vec3b srcPixel = srcMatImg.at<cv::Vec3b>(i,j);
            cvSet2D(*destIplImg, i, j, Scalar(srcPixel[0],srcPixel[1],srcPixel[2]) );
            //printf("%d %d %d\n",srcPixel[0],srcPixel[1],srcPixel[2]);
        }
    }
    typeLog.printLog("check thread CopyMat2Ipl() exit");
}
