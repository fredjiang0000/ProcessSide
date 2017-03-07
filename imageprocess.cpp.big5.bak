#include "stdafx.h"
#include "header.h"

using namespace cv;
using namespace std;

const double Radian2Angle_Ratio = 180.0f/3.141592f;

/*********************************************************
*〔Function〕雜訊處理程序
*〔參數〕
*	srcImg		:	原始影像
*	destImg		:	處理後的目標影像
**********************************************************/
void clearImgNoise(IplImage *srcImg,IplImage *destImg) {
    //先用〔高斯濾波器〕做處理
    cvSmooth(srcImg,destImg,CV_GAUSSIAN,5,5);
}

/*******************************************************************
*[Function] 彩色影像之遮罩ＡＮＤ運算：
*			若mask pixel的值 ==	０（設定黑色）；
*							 !=	０（保留原來pixel）
*〔參數〕
*	srcImg	:	原始影像
*	mask	:	遮罩影像（背景＝０；	前景＝	非０）
*	destImg	:	遮罩處理後的影像
*
********************************************************************/
void colorAND(IplImage *srcImg,IplImage *mask,IplImage *destImg){
    for( int i = 0; i < mask->height; i++ ){
        for( int j = 0; j < mask->width; j++ ){
            CvScalar scalar = cvGet2D(mask, i, j);

            if(scalar.val[0] == 0){
                cvSet2D(destImg, i, j, cvScalar( 0, 0, 0));
            } else
                cvSet2D(destImg, i, j, cvGet2D(srcImg, i, j) );
        }
    }
}

/*******************************************************************
*[Function] 以〔方形區域〕做為參數來繪製出一個矩形
********************************************************************/
void drawRectangle(IplImage *destImg,CvRect rect,CvScalar color,int thickness,int type){
    cvRectangle(	destImg,
                    cvPoint(rect.x,rect.y),
                    cvPoint(rect.x + rect.width,rect.y+rect.height),
                    color,thickness,type
                );
}

/********************************************************
*[Function] 計算向量ＯＡ與ＯＢ之間的〔夾角〕
*[運算公式]:
*	Ｃｏｓθ	　		  （ＯＡ‧ＯＢ）
*				＝＞	＿＿＿＿＿＿＿＿＿
*						｜ＯＡ｜ｘ｜ＯＢ｜
*
*
*									（Ｘ１＊Ｘ２＋Ｙ１＊Ｙ２）
*				＝＞	＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
*						（sqrt（Ｘ１^2＋Ｙ１^2）＋sqrt（Ｘ２^2＋Ｙ２^2））
********************************************************/
double getAngle(CvPoint &pointO, CvPoint& pointA, CvPoint& pointB){
    CvPoint vectorOA = cvPoint(pointA.x - pointO.x,pointA.y - pointO.y);		//ＯＡ向量
    CvPoint vectorOB = cvPoint(pointB.x - pointO.x,pointB.y - pointO.y);		//ＯＢ向量

    //向量的長度
    double vectorOA_Len = sqrt( (double) (vectorOA.x * vectorOA.x + vectorOA.y * vectorOA.y) );
    double vectorOB_Len = sqrt( (double) (vectorOB.x * vectorOB.x + vectorOB.y * vectorOB.y) );

    //兩向量〔內積〕
    double dot = vectorOA.x * vectorOB.x + vectorOA.y * vectorOB.y;

    //計算的結果為〔Cos〕
    double cosV = dot / (vectorOA_Len * vectorOB_Len);

    //因此最後結果還需要再轉換成〔角度〕
    return cos2angle(cosV);
}

/********************************************************
*[Function] 將〔Ｃｏｓ的值〕轉換成〔角度〕
********************************************************/
double cos2angle(const double &cosValue){
    return acos(cosValue) * Radian2Angle_Ratio;
}

/********************************************************
*[Function] ３ｘ３的行列式運算
*[運算公式]:
*		|Ｘ１　Ｙ１　１|
*		|Ｘ２　Ｙ２　１|
*		|Ｘ３　Ｙ３　１|
*
* =>　（Ｘ１＊Ｙ２　＋　Ｘ２＊Ｙ３　＋　Ｘ３＊Ｙ１）
*   －（Ｘ１＊Ｙ３　＋　Ｘ２＊Ｙ１　＋　Ｘ３＊Ｙ２）
********************************************************/
double determinant_3x3(
                        double x1, double y1,
                        double x2, double y2,
                        double x3, double y3
){
    return	(x1 * y2 + x2 * y3 + x3 * y1) -
            (x1 * y3 + x2 * y1 + x3 * y2) ;
}

/********************************************************
*[Function] 以ＯＡ、ＯＢ兩向量做外積
*[定理]:
*	若回傳的值為： ＝　0,ＯＡ、ＯＢ位在同一條直線上
*			　		＞　0,ＯＡ往ＯＢ為［順時針］方向
*					＜　0,ＯＡ往ＯＢ為［逆時針］方向
*
*［功能］：可以用來判斷兩條直線之間的相對方向（e.g.從 [左邊] 或從 [右邊] 過來的線段）
*
********************************************************/
double crossProduct(CvPoint pointO, CvPoint pointA, CvPoint pointB) {
    return determinant_3x3(	pointA.x	,	pointA.y	,
                            pointB.x	,	pointB.y	,
                            pointO.x	,	pointO.y	);
}

/*******************************************************************
*[Function] 只針對某色彩色ＲＧＢ顏色的二值化處理
*			若某個pixel的顏色為bgColor => 設定為：０（黑色）
*								  否則 => 設定為：255（白色）
*〔參數〕
*	srcImg	:	原始影像（彩色）
*	destImg	:	二值化後的目標影像（灰階）
*	bgColor	:	判斷二值化的目標顏色（ＲＧＢ）
*
********************************************************************/
void bgThreshold(IplImage *srcImg,IplImage *destImg,Scalar bgColor){
    for( int i = 0; i < srcImg->height; i++ ){
        for( int j = 0; j < srcImg->width; j++ ){
            CvScalar scalar = cvGet2D(srcImg, i, j);

            //只有當ＲＧＢ的顏色皆與目標顏色一樣時，才設定為〔背景〕（黑色）
            if(	scalar.val[0] == bgColor.val[0] &&
                scalar.val[1] == bgColor.val[1] &&
                scalar.val[2] == bgColor.val[2]
            ){
                cvSet2D(destImg, i, j, cvScalar( 0, 0, 0));
            } else
                //否則，一律當作是前景（白色）
                cvSet2D(destImg, i, j, cvScalar(255,255,255) );
        }
    }
}

