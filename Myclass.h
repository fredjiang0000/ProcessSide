/*********************************************************
*〔自訂型態區塊〕：
**********************************************************/
#ifndef __MYCLASS_H__
#define __MYCLASS_H__

#include "stdafx.h"
#include "header.h"

using namespace std;



//〔Class 原型宣告〕：
class HandEdge;
class HandJoint;
class HandArea;

/*********************************************************
*〔Class〕手部物件
**********************************************************/
class HandArea{
    public:
        vector<HandJoint> joints;	//存放手的〔關節點〕之Vector
        CvSeq *contour;				//手部輪廓
        CvSeq *approContour;		//手部輪廓(By 近似曲線)

        CvRect rectArea;			//手部最小方形區塊
        CvPoint center;				//手部質心座標
        double isInDistance;		//質心到手部的最小輪廓距離
                                    //（ＰＳ：若為正數：質心在輪廓內部	；
                                    //			  負數：質心在輪廓外部	）

        //整隻手部關節點的〔統計資訊〕：
        int outsideJointCount;		//有出現 "外角' 的關節點 [個數]
        HandJoint *lowerJoint;		//整個手部〔最低點〕的關節（但在影像中的Ｙ軸為最大）
        HandJoint *higherJoint;		//整個手部〔最高點〕的關節（但在影像中的Ｙ軸為最小）

        //〔手上握東西〕的相關變數：
        bool isFindHoldArea;		//該手部上 "是否存在" 握有東西的區域
        CvSeq *holdContour;			//手握東西的〔輪廓〕
        CvPoint holdCenter;			//手握東西輪廓的〔質心〕
        double holdCenterDist;		//手握東西的〔輪廓到質心〕的距離
        int holdLeftIndex;			//握東西的輪廓在近似曲線的〔左邊索引〕邊界
        int holdRightIndex;			//握東西的輪廓在近似曲線的〔右邊索引〕邊界

        /*********************************************************
        *〔Function〕Constructor
        **********************************************************/
        HandArea(CvSeq *theContour){
            this->contour = theContour;
            this->isFindHoldArea = false;
            this->holdLeftIndex = this->holdRightIndex = -1;

        }
};

/*********************************************************
*〔Class〕關節點的〔線段〕
**********************************************************/
class HandEdge{
    public:
        //〔成員變數區塊〕：
        CvPoint point1;			//起點
        CvPoint point2;			//終點

        //〔成員變數區塊〕：
        HandEdge(CvPoint p1,CvPoint p2);
        HandEdge();
};

/*********************************************************
*〔Class〕手部輪廓上的〔關節點〕資訊
**********************************************************/
class HandJoint{
    public:
        //〔成員變數區塊〕：
        int index;			//關節點的編號
        CvPoint point;		//該關節點的座標（點Ｏ）
        HandEdge edge1;		//線段ＯＡ
        HandEdge edge2;		//線段ＯＢ

        double angle;		//線段ＯＡ與ＯＢ之間的夾角
        bool isObtuse;		//該夾角是否為〔鈍角〕
        bool isOutside;		//該夾角是否為〔外角〕
        bool isHold;		//標記該關節點是否可能握有物體(手機)

        //〔成員函式區塊〕：
        HandJoint();
        HandJoint(HandEdge edge1,HandEdge edge2,double theAngle = 0.0f,double cross = 0.0f);
};

//其他相關函式
const int HOLD_DOUBLE_CONCAVE = 1;					//雙手握物
const int HOLD_SINGLE_UPPER = 2;					//單手上半部握物

static int seqComparable( const void* _point1, const void* _point2, void* userdata);

bool findHoldEdge(HandArea &hand);
bool doubleHandCheck(vector<HandArea *> &hands);
bool singleHandCheck(vector<HandArea *> &hands);
bool getUpperHand(HandArea &hand,int &leftIndex,int &rightIndex);
bool getGraspHand(HandArea &hand,int &leftIndex,int &rightIndex);

void holdHandUpdate(HandArea *hand,int action);
bool getHoldMask(IplImage *handImg,IplImage *maskImg,vector<HandArea *> &hands);

#endif
