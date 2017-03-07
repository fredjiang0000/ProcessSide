#include "stdafx.h"
#include "header.h"

using namespace std;
using namespace cv;

/************************************************************************/
/*[Function] 再進行Ｓｏｒｔ時,比較大小的處理程序
/************************************************************************/
static int seqComparable( const void* _point1, const void* _point2, void* userdata){
    CvPoint* point1 = (CvPoint*)_point1;
    CvPoint* point2 = (CvPoint*)_point2;

    int y_diff = point1->y - point2->y;
    int x_diff = point1->x - point2->x;
    return y_diff ? y_diff : x_diff;
}

/************************************************************************/
/*[Function] Default Constructor
/************************************************************************/
HandEdge::HandEdge(){
    this->point1 = cvPoint(0,0);
    this->point2 = cvPoint(0,0);
}

/************************************************************************/
/*[Function] Constructor
/*〔參數〕
/*	p1		:	線段的〔起點〕
/*	p2		:	線段的〔終點〕
/*
/************************************************************************/
HandEdge::HandEdge(CvPoint p1,CvPoint p2){
    this->point1 = cvPoint(p1.x,p1.y);
    this->point2 = cvPoint(p2.x,p2.y);
}

/************************************************************************/
/*[Function] Constructor
/*〔參數〕
/*	edge1		:	ＯＡ的向量
/*	edge2		:	ＯＢ的向量
/*	theAngle	:	ＯＡ、ＯＢ兩向量的〔夾角〕
/*	cross		:	ＯＡ、ＯＢ兩向量的〔外積〕
/*
/************************************************************************/
HandJoint::HandJoint(HandEdge edge1,HandEdge edge2,double theAngle,double cross){
    this->point = edge1.point1;
    this->edge1 = edge1;
    this->edge2 = edge2;
    this->angle = theAngle;
    this->isHold = false;

    //若夾角大於９０度，則標記為〔鈍角〕
    this->isObtuse = (this->angle > 90);

    //若外積的結果為負值，代表該夾角是一個〔外角〕
    this->isOutside = (cross < 0.0f);
}

/************************************************************************/
/*[Function] Default Constructor
/************************************************************************/
HandJoint::HandJoint(){	}

/************************************************************************/
/*[Function] 找尋手部輪廓中，可能出現拿手機的區域
/*〔參數〕
/*	hand	:	紀錄手部各種資訊的物件
/*
/************************************************************************/
bool findHoldEdge(HandArea &hand){
    int pointNum = hand.approContour->total;	//〔關節點個數〕為節點的個數
    hand.isFindHoldArea = false;				//判斷到目前為止是否有找到可能握有手機的區域

    //〔統計變數〕：
    hand.outsideJointCount = 0;					//出現外角的總個數
    int minH = 999999,maxH = -1;				//手部的 [最高點] 與 [最低點] 的關節
    int minIndex,maxIndex;

    /************************************************************************/
    /*	將每個節點轉換成〔關節點〕資訊：夾角、是否鈍角、是否為外角
    /************************************************************************/
    for (int i = 0; i < pointNum; i++) {
        CvPoint pointO,pointA,pointB;

        //每次都以該點為中心(Ｏ點)，之後在各別以上一個與下一個節點為Ａ點、Ｂ點，來判斷兩線段的夾角關係
        pointO = *(CvPoint*) cvGetSeqElem ( hand.approContour, i );
        pointA = *(CvPoint*) cvGetSeqElem ( hand.approContour, (i+pointNum-1) % pointNum );
        pointB = *(CvPoint*) cvGetSeqElem ( hand.approContour, (i+1)			 % pointNum );

        double angle = getAngle(pointO,pointA,pointB);			//計算ＯＡ、ＯＢ兩向量的〔夾角〕
        double cross = crossProduct(pointO,pointA,pointB);		//計算ＯＡ、ＯＢ兩向量的〔外積〕

        //產生關節點資訊
        HandJoint joint = HandJoint(HandEdge(pointO,pointA),HandEdge(pointO,pointB),angle,cross );

        //順便標記該關節點的 [編號]
        joint.index = i;

        if(joint.isOutside == true)
        ++hand.outsideJointCount;

        /************************************************************************/
        /* 更新目前手部最低點與最高點的資料
        /************************************************************************/
        if(minH > pointO.y){
            minH = pointO.y;
            hand.lowerJoint = &joint;
            minIndex = i;
        }

        if(maxH < pointO.y){
            maxH = pointO.y;
            hand.higherJoint = &joint;
            maxIndex = i;
        }

        //最後,在加入Vector中
        hand.joints.push_back(joint);
    }

    //記錄整隻手部的〔最高點〕與〔最低點〕
    hand.lowerJoint = &hand.joints[maxIndex];
    hand.higherJoint = &hand.joints[minIndex];

    //printf("[%d] 最高點 = %d\t最低點 = %d\n",
    //		hand.outsideJointCount,hand.higherJoint->index,hand.lowerJoint->index);

    /************************************************************************/
    /* 依序檢查哪些關節點可能有〔握手機〕
    /************************************************************************/
    for (int i = 0; i < hand.joints.size(); i++) {
        //printf("[%d] = %0.lf (%s)\n",i,joints[i].angle,(joints[i].isOutside == true)?"逆":"順" );

        //版本二：直接去找〔連續性的兩個外角〕 或 〔兩個外角中間夾一個內角〕 的區域
        if( hand.joints[i].isOutside == true &&
            (	hand.joints[(i+1) % pointNum].isOutside ||
                hand.joints[(i+2) % pointNum].isOutside )
        ) {
            int leftIndex = i;											//紀錄〔左邊〕邊界
            int rightIndex = (hand.joints[(i+1) % pointNum].isOutside)? //紀錄〔右邊〕邊界
                                i + 1	:	i + 2;

            /************************************************************************/
            /* 之後再將右邊界向〔右邊延伸〕
            /************************************************************************/
            do{
                int nextRightIndex = rightIndex;
                int nextIndex1 = (rightIndex + 1) % pointNum;
                int nextIndex2 = (rightIndex + 2) % pointNum;

                if(hand.joints[nextIndex1].isOutside) rightIndex = nextIndex1;
                if(hand.joints[nextIndex2].isOutside) rightIndex = nextIndex2;

                //更新檢查的索引 (因為之前已標記過的連續性關節點(外角),已經不需要再做檢查)
                i = (i < (rightIndex+1) % pointNum)? (rightIndex+1) % pointNum : i;

                //索引沒再更新時,就結束〔向外延伸〕處理程序
                if(nextRightIndex == rightIndex) break;

            } while(true);

            /************************************************************************/
            /* 同時也將左邊界向〔左邊延伸〕
            /************************************************************************/
            do {
                int nextLeftIndex = leftIndex;
                int nextIndex1 = (leftIndex - 1 + pointNum) % pointNum;
                int nextIndex2 = (leftIndex - 2 + pointNum) % pointNum;

                if(hand.joints[nextIndex1].isOutside) leftIndex = nextIndex1;
                if(hand.joints[nextIndex2].isOutside) leftIndex = nextIndex2;

                //索引沒再更新時,就結束〔向外延伸〕處理程序
                if(nextLeftIndex == leftIndex) break;

            } while(true);

            //之後,會再度將〔左／右兩邊界〕各自向外延伸一個關節點（內角）
            rightIndex = (rightIndex + 1) % pointNum;
            leftIndex =  (leftIndex  - 1 + pointNum) % pointNum;

            /************************************************************************/
            /* 最後,再將這些邊界內的關節點標記成手上〔握有物體〕的區域
            /************************************************************************/
            for(int holdIndex = leftIndex;
                (holdIndex) % pointNum != (rightIndex + 1) % pointNum;
                holdIndex++
            ) {
                hand.joints[(holdIndex) % pointNum].isHold = true;
            }

            //之後，再去記錄最終〔左／右兩邊界〕的索引
            hand.holdLeftIndex = leftIndex;
            hand.holdRightIndex = rightIndex;
            //printf("%d ~ %d\n",leftIndex,rightIndex);
        }
    }

    //若左、右兩邊界的索引皆為 -1,代表沒有找到可能握有手機的區域
    hand.isFindHoldArea = !(hand.holdLeftIndex == -1 && hand.holdRightIndex == -1);

    //若找不到,則直接離開處理程序
    if(!hand.isFindHoldArea) return false;

    {	/************************************************************************/
        /*之後，繼續計算可能出現手機區域的輪廓：找尋輪廓〔質心〕、〔距離〕
        /*找尋原始輪廓(點集合)中,找尋左、右邊界兩點的〔點索引〕
        /*以便之夠能以延著輪廓區域繪製〔確定前景〕的線條
        /************************************************************************/
        //printf("連續標記區域：%d－%d\n",hand.holdLeftIndex,hand.holdRightIndex);

        CvPoint *leftPoint = &hand.joints[hand.holdLeftIndex % pointNum].point,		//〔左邊邊界〕在近似曲線上的節點座標
                *rightPoint = &hand.joints[hand.holdRightIndex % pointNum].point;	//〔右邊邊界〕在近似曲線上的節點座標

        int leftContourIndex,		//左邊邊界在原始輪廓上的〔點索引〕
            rightContourIndex;		//右邊邊界在原始輪廓上的〔點索引〕

        cvSeqSearch(hand.contour,leftPoint,seqComparable,false,&leftContourIndex);			//找尋左邊界的〔點索引〕
        cvSeqSearch(hand.contour,rightPoint,seqComparable,false,&rightContourIndex);			//找尋右邊界的〔點索引〕
        //printf("點索引：%d － %d [%d]\n",leftContourIndex,rightContourIndex,hand.contour->total);

        //取得輪廓的〔子序列〕(手上可能握有物品的區域)
        hand.holdContour = cvSeqSlice(hand.contour,cvSlice(leftContourIndex,rightContourIndex),storage,0);

        //並且找尋輪廓的〔質心〕
        getCenter(hand.holdContour,hand.holdCenter);

        //以及質心到輪廓的〔最短距離〕
        hand.holdCenterDist = cvPointPolygonTest(hand.holdContour,Point2Point32f(hand.holdCenter),true);
    }

    return true;
}

/************************************************************************/
/*[Function] 檢查握拳的手勢 (取得「手腕到最高點」的輪廓)
/*[參數]
/*	hand		:	手部物件
/*	leftIndex	:	握物區域的左邊邊界
/*	rightIndex	:	握物區域的右邊邊界
/*
/************************************************************************/
bool getGraspHand(HandArea &hand,int &leftIndex,int &rightIndex){
    vector<HandJoint *> candidateJoints;		//記錄哪一些是〔外角〕的關節點集合
    int numJoints = hand.joints.size();

    /************************************************************************/
    /* 先去記錄哪一些關節點是〔外角〕
    /************************************************************************/
    for(int i = 0;i < numJoints;i++) {
        if(hand.joints[i].isOutside == true)
        candidateJoints.push_back(&hand.joints[i]);
    }


    /************************************************************************/
    /* 之後再去找尋〔手腕〕的關節點
    /************************************************************************/
    int wristIndex = -1;									//〔手腕〕關節點的索引

    for(int i = 0;i < candidateJoints.size();i++){
        bool isLeft;										//候選鍵的關節點到最高點的 [方向]
        int candidateIndex = candidateJoints[i]->index;		//要檢查的關節點索引

        /************************************************************************/
        /* 先檢查該點到最高點的〔方向〕為何
        /************************************************************************/
        for(int j = candidateIndex + 1;;j++){
            if(	j % numJoints == hand.higherJoint->index){
                isLeft = false;
                break;

            } else if( j % numJoints == hand.lowerJoint->index	){
                isLeft = true;
                break;
            }
        }

        //printf("方向 = %s\n",isLeft? "左邊" : "右邊");

        int obtuseCount = 0;					//「該關節點至最高點」之間出現〔滿足條件〕的關節點個數
        int indexOffset = (isLeft)? -1 : +1;	//每次要檢查下一個關節點的偏移量 (左邊 | 右邊)

        for(	int i = indexOffset;
                (candidateIndex + i + numJoints) % numJoints != hand.higherJoint->index ;
                i += indexOffset
        ) {
            //先計算要檢查的關節點索引
            int index = (candidateIndex + i + numJoints) % numJoints;

            if(	hand.joints[index].isObtuse == true ||				//若是出現連續性的〔鈍角〕
                hand.joints[index].angle >= 70.0f ||				//或是該角度是〔接近鈍角〕
                (	(hand.joints[index].isOutside) &&				//或者是在「大拇指與食指」之間的關節點
                    (index + indexOffset + numJoints) % numJoints == hand.higherJoint->index
                )
            ) {
                obtuseCount++;

            //不滿足上述條件，就斷開檢查程序
            } else {
                break;
            }
        }

        //若出現連續兩個以上的關節點滿足上述條件，就直接被判斷為〔手腕〕上的關節點
        if(obtuseCount >= 2){
            wristIndex = candidateIndex;

            leftIndex = (!isLeft)? wristIndex : hand.higherJoint->index;
            rightIndex = (isLeft)? wristIndex : hand.higherJoint->index;
            break;
        }
    }

    debugPrintf("手腕 = %d [%d ~ %d]\n",wristIndex,leftIndex,rightIndex);
    return (wristIndex != -1);
}

/************************************************************************/
/*[Function] 檢查手的上半部做為可能性的區域
/*[參數]
/*	hand		:	手部物件
/*	leftIndex	:	握物區域的左邊邊界
/*	rightIndex	:	握物區域的右邊邊界
/*
/************************************************************************/
bool getUpperHand(HandArea &hand,int &leftIndex,int &rightIndex){
    int numPoints = hand.joints.size();
    double leftSlope,rightSlope;				//左右點邊向外延伸一個關節點的〔斜率〕

    //各自計算最高點「向外延伸左、右一個」的關節點
    int upLeftIndex = (hand.higherJoint->index - 1 + numPoints) % numPoints,
        upRightIndex = (hand.higherJoint->index + 1) % numPoints;

    HandJoint	*maxLeftJoint = &hand.joints[upLeftIndex],
                *maxRightJoint = &hand.joints[upRightIndex];

    //藉由比較左右兩邊的斜率,判斷手部較為 [平坦] 的區域在哪裡
    leftSlope = getSlope(hand.higherJoint->point,maxLeftJoint->point);
    rightSlope = getSlope(hand.higherJoint->point,maxRightJoint->point);

    debugPrintf("左右兩邊斜率 = %lf,%lf\n",leftSlope,rightSlope);
    bool isLeft = (fabs(leftSlope) < fabs(rightSlope) );

    /************************************************************************/
    /* 之後,便是將手部平坦的區域連接起來
    /************************************************************************/
    HandJoint *prevJoint,*nextJoint;
    prevJoint = hand.higherJoint;

    while(true){
        int nextIndex = (isLeft == true)?
            (prevJoint->index - 1 + numPoints) % numPoints	:
            (prevJoint->index + 1) % numPoints;

        nextJoint = &hand.joints[nextIndex];
        double slope = fabs(getSlope(prevJoint->point,nextJoint->point) );
        debugPrintf("斜率 = %lf [%d ~ %d]\n",slope,prevJoint->index,nextJoint->index);

        //斜率變化的門檻值(需要一直維持平坦的狀態)
        if( slope > 0.5)	break;

        prevJoint = nextJoint;
    }

    //最後更新左,右兩邊的邊界
    leftIndex = (!isLeft)? hand.higherJoint->index : prevJoint->index;
    rightIndex = (isLeft)? hand.higherJoint->index : prevJoint->index;

    //若左右兩邊的邊界所引相同時,代表找尋失敗
    return (leftIndex != rightIndex);
}

/************************************************************************/
/*[Function] 更新手部握物的區域紀錄
/*[參數]
/*	hand		:	手部物件
/*	action		:	更新方式
/*
/************************************************************************/
void holdHandUpdate(HandArea *hand,int action){
    int numPoints = hand->joints.size();

    /************************************************************************/
    /* 先將左、右兩邊界之間的關節點標記為握物的區域
    /************************************************************************/
    for(int i = hand->holdLeftIndex;
        (i % numPoints) != (hand->holdRightIndex + 1) % numPoints ;
        i++
    ){
        hand->joints[i % numPoints].isHold = true;
    }

    //標記該手部已存在可能有握物的區域
    hand->isFindHoldArea = true;


    //找尋可能握有物品區域的〔子輪廓〕、〔質心〕、〔距離〕
    int leftContourIndex,		//左邊邊界在原始輪廓上的〔點索引〕
        rightContourIndex;		//右邊邊界在原始輪廓上的〔點索引〕

    CvPoint *leftPoint = &hand->joints[hand->holdLeftIndex].point,
            *rightPoint = &hand->joints[hand->holdRightIndex].point;

    cvSeqSearch(hand->contour,leftPoint,seqComparable,false,&leftContourIndex);			//找尋左邊界的〔點索引〕
    cvSeqSearch(hand->contour,rightPoint,seqComparable,false,&rightContourIndex);			//找尋右邊界的〔點索引〕

    //紀錄手上可能握有物品輪廓的〔子輪廓〕
    hand->holdContour = cvSeqSlice(hand->contour,cvSlice(leftContourIndex,rightContourIndex),storage,0);

    /************************************************************************/
    /* 依照手部握物的方式，做不一樣的更新方式
    /************************************************************************/
    switch(action){
        //雙手握物：則質心點為該輪廓的質心（中心）位置
        case HOLD_DOUBLE_CONCAVE:
        {
            //以及找尋輪廓上的〔質心點〕
            getCenter(hand->holdContour,hand->holdCenter);
            break;
        }

        //手的上半部握物：則質心點會往上偏移（Ｙ軸）一些
        case HOLD_SINGLE_UPPER:
        {
            //依照手上半部的輪廓之〔周長〕，去計算質心往上偏移的〔Ｙ軸偏移量〕
            int heightOffset = ( (int) cvContourPerimeter(hand->holdContour) ) >> 3;

            //並且去計算手上半部的中心點座標
            CvPoint midPoint = cvPoint( (	(leftPoint->x + rightPoint->x) >> 1	)					,
                                        (	(leftPoint->y + rightPoint->y) >> 1 ) - heightOffset	);

            hand->holdCenter = midPoint;

            //檢查座標是否有超出邊界範圍
            if(hand->holdCenter.y < 0) hand->holdCenter.y = 0;

            break;
        }
    }

    //與質心到輪廓的〔最短距離〕
    hand->holdCenterDist = cvPointPolygonTest(hand->holdContour,Point2Point32f(hand->holdCenter),true);
}

/************************************************************************/
/*[Function] 單手握機情形的檢查處理程序
/*[參數]
/*	hands	:	紀錄手部各種資訊的物件之Vector
/*
/************************************************************************/
bool singleHandCheck(vector<HandArea *> &hands){
    //只處理單手的狀況
    if( !(	hands.size() == 1 && !hands[0]->isFindHoldArea) )	return false;

    HandArea *hand = hands[0];				//紀錄要處理的手部
    int numPoints = hand->joints.size();	//關節個數

    bool isUpdate;							//紀錄是否有找到其他可能握物的區域
    int leftIndex,rightIndex;				//握物區域的左右兩邊界

    /************************************************************************/
    /* 依照〔外角個數〕做相對應的檢查程序
    /************************************************************************/
    switch(hand->outsideJointCount){
        /************************************************************************/
        /*	若不存在任何外角(沒有做出彎曲的動作)：
        /*	則去找尋〔手的上半部〕做為可能性的區域
        /************************************************************************/
        case 0:
        {
            isUpdate = getUpperHand(*hand,leftIndex,rightIndex);
            break;
        }

        /************************************************************************/
        /* 否則，就找尋〔手腕〕到〔手部最高點〕的輪廓區域
        /************************************************************************/
        default:
        {
            isUpdate = getGraspHand(*hand,leftIndex,rightIndex);
            break;
        }
    }

    /************************************************************************/
    /* 若有找到新的區域，才去做更新處理
    /************************************************************************/
    if(isUpdate){
        hand->holdLeftIndex = leftIndex;
        hand->holdRightIndex = rightIndex;

        holdHandUpdate(hand,HOLD_SINGLE_UPPER);
    }

    return isUpdate;
}

/************************************************************************/
/*[Function] 檢查雙手握有物品的情形
/*〔參數〕
/*	hands	:	紀錄手部各種資訊的物件之Vector
/*
/************************************************************************/
bool doubleHandCheck(vector<HandArea*> &hands){
    /************************************************************************/
    /* 只去檢查存在雙手的區域
    /* 並且是有一隻手已經有繪製可能的區域，但另外一隻手卻沒有繪製
    /************************************************************************/
    if(	 hands.size() == 1 ||
        (hands.size() == 2 && (!hands[0]->isFindHoldArea && !hands[1]->isFindHoldArea) )	)
    return false;

    HandArea *hand1 = (hands[0]->isFindHoldArea)? hands[0] : hands[1];		//〔已存在標示〕的手部
    HandArea *hand2 = (hand1 == hands[0])? hands[1] : hands[0];				//〔不存在標示〕的手部

    /************************************************************************/
    /* 先取得已存在標記手部的輪廓〔最高點〕與〔最低點〕
    /************************************************************************/
    int minH = 999999,
        maxH = -1;

    for(	int i = hand1->holdLeftIndex, N = hand1->approContour->total;
            (i % N) != (hand1->holdRightIndex + 1) % N;
            i++
    ){
        int pointH = hand1->joints[i % N].point.y;

        //檢查高度資訊是否有要更新
        if(minH > pointH)	minH = pointH;
        if(maxH < pointH)	maxH = pointH;
    }

    vector <int> indexSet;
    /************************************************************************/
    /* 接著，再去把那些未做任何標記的另一隻手中
    /* 找出是〔外角〕，且座邊點的高度是落在另一隻手部輪廓的範圍內
    /* 作為"候選"參考的節點
    /************************************************************************/
    for(int i = 0,N = hand2->approContour->total;i < N;i++){
        HandJoint *joint = &hand2->joints[i];

        //滿足條件,就加入到Vector中
        if(	joint->isOutside == true							&&
            minH <= joint->point.y && joint->point.y <= maxH	)
        indexSet.push_back(i);
    }

    /************************************************************************/
    /* 之後,再從這些參考節點的集合中
    /* 找尋離具有標記的手部〔最近的節點〕（By Ｘ座標的差值）
    /* 做為是未標記手部的可能存在握有物品的關節點
    /************************************************************************/
    int minIndex = -1;

    for(	int i = 0,minDist = 99999;
            i < indexSet.size();
            i++
    ) {
        HandJoint *joint =	&hand2->joints[indexSet[i]];

        //計算該節點(未標記) 到 另一隻手上可能存在物品區域的質心 [距離]
        double dist = fabs((double)joint->point.x - hand1->holdCenter.x);

        if(minDist > dist){
            minDist = dist;
            minIndex = indexSet[i];
        }
    }

    /************************************************************************/
    /* 更新未標記手的相關資訊
    /************************************************************************/
    int hand2_JointNum = hand2->joints.size();
    hand2->holdLeftIndex = (minIndex - 1 + hand2_JointNum) % hand2_JointNum;	//紀錄左邊邊界
    hand2->holdRightIndex = (minIndex + 1) % hand2_JointNum;					//紀錄右邊邊界

    holdHandUpdate(hand2,HOLD_DOUBLE_CONCAVE);
}

/************************************************************************/
/*[Function] 取得手部可能握有物品的遮罩
/*〔參數〕
/*	handImg		:	手部區域的影像（灰階）
/*	maskImg		:	要存放遮罩的目標影像（灰階）
/*	hands	:	紀錄手部各種資訊的物件之Vector
/*
/************************************************************************/
bool getHoldMask(IplImage *handImg,IplImage *maskImg,vector<HandArea *> &hands){
    int numHand = hands.size();

    {	/************************************************************************/
        /* 計算〔背景遮罩〕的數值
        /* 只要有一隻手存在可能的區域，就將〔背景遮罩〕設定為〔預期背景〕；
        /* 否則，設定為〔預期前景〕
        /************************************************************************/
        bool isExistArea = false;

        for(int i = 0;i < numHand;i++){
            //只有一隻手存在可能的區域,就將〔背景遮罩〕設定為〔預期背景〕
            if(hands[i]->isFindHoldArea){
                isExistArea = true;
                break;
            } else {
                return false;
            }
        }

        //處理找不到的情形
        if(isExistArea == false)	return false;

        //int bgMask = (isExistArea)? cv::GC_PR_BGD : cv::GC_PR_FGD;
        int bgMask = cv::GC_PR_BGD;

        //設定〔背景區域〕的遮罩
        drawRectangle(maskImg,cvRect(0,0,handImg->width,handImg->height),Scalar(bgMask),CV_FILLED);
    }

    /************************************************************************/
    /* 之後，再去檢查每一隻手，沿著手上可能的區域去繪製線條
    /************************************************************************/
    for(int i = 0;i < numHand;i++) {
        if(numHand>1)
            return false;
        //先繪製手部握物區域上的質心
        cvDrawCircle(maskImg,hands[i]->holdCenter,fabs(hands[i]->holdCenterDist),Scalar(cv::GC_FGD), CV_FILLED );

        if(hands[i]->holdCenter.x < 0 || hands[i]->holdCenter.y < 0)
        return false;

        debugPrintf("質心：(%d,%d) [%.2lf]\n",
                    hands[i]->holdCenter.x,hands[i]->holdCenter.y,
                    hands[i]->holdCenterDist						);

        //計算點集合的〔邊界長度〕

        cout<<"No BUG"<< numHand << " "<<hands.size()<<endl;
        cout<<"hand "<<i<<" : "<<hands[i]->holdContour->total<<endl;
        int lengthContour = hands[i]->holdContour->total;			//會有Bug
        cout<<"No BUG"<<endl;
        //要捨棄掉〔最前面〕、〔最後面〕N個點的個數，避免在輪廓繪製上會抓到其他雜訊
        int discardSize = lengthContour >> 3;
        debugPrintf("輪廓長度：%d\t捨棄長度：%d\n",lengthContour,discardSize);

        /************************************************************************/
        /* 接著便是沿著〔左邊邊界〕到〔右邊邊界〕的輪廓上繪製〔確定前景〕的遮罩
        /************************************************************************/
        for(int j = 0;j < lengthContour;j++){

            //在最前面、最後面的Ｎ個點做捨棄，所以不進行繪製
            if(	(j < discardSize)					||
                (lengthContour - j) < discardSize	)
            continue;

            //在該〔輪廓點〕上繪製一個「圓」（確認前景）的區域
            CvPoint point =	*(CvPoint*) cvGetSeqElem (hands[i]->holdContour, j);
            cvDrawCircle(maskImg, point, 3, Scalar(cv::GC_FGD), CV_FILLED);
        }
    }

    /************************************************************************/
    /* 若存在兩隻手，則以兩手的質心去繪製一條〔直線〕，來增加準確度
    /************************************************************************/
    if(numHand == 2 && (hands[0]->isFindHoldArea && hands[1]->isFindHoldArea) ){
        int lineSize = (hands[0]->holdCenterDist + hands[1]->holdCenterDist) / 2;
        cvDrawLine(maskImg,hands[0]->holdCenter,hands[1]->holdCenter,Scalar(cv::GC_FGD),lineSize,CV_FILLED);
    }

    {	/************************************************************************/
        /* 最後，再將〔手部區域〕的輪廓做為〔確定背景〕，以表示膚色的區域不去做考量
        /************************************************************************/
        IplImage *tempImg = cvCreateImage(cvGetSize(handImg),8,1);

        //之後再與原來〔手部輪廓〕的區域做 [合併]
        cvThreshold(handImg, tempImg, 0, 255, CV_THRESH_BINARY_INV);
        cvAnd(maskImg,tempImg,maskImg);
        cvErode(maskImg,maskImg,0,1);

        //cvShowImage("手的遮罩",handImg);
        //cvShowImage("手機遮罩",maskImg);
        cvReleaseImage(&tempImg);
    }

    {	/************************************************************************/
        /*				檢查「確定前景」的Pixel數目是否到達一定門檻
        /************************************************************************/
        const double targetLabel = cv::GC_FGD;		//檢查目標
        int count = 0;

        for(int i = 0;i < maskImg->height;++i) {
            for(int j = 0;j < maskImg->width;++j) {
                const Scalar scalar = cvGet2D(maskImg, i, j);

                //目前Pixel與檢查目標相符,就增加計數
                if(scalar.val[0] == targetLabel ){
                    ++count;

                    //到達門檻時,就確認是滿足GrabCut條件
                    if(count >= 10) return true;
                }
            }
        }
    }

    return false;
}
