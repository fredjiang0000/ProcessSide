#include "stdafx.h"
#include "header.h"
#include "outputLog.h"

using namespace cv;
using namespace std;

outputLog cellDetectLog("cellDetect.log");
/************************************************************************/
/*[Function] 偵測手部有無握住一個〔方形物體〕
/*			 若有則回傳True;	否則回傳False
/*[參數]：
/*	srcImg		:	原始〔待測影像〕	(彩色)
/*	destImg		:	偵測〔結果影像〕	(彩色)
/*
/************************************************************************/
bool cellDetect(const Mat &srcImgMat){

     cellDetectLog.printLog("check thread cellDetect() enter");
//bool cellDetect(IplImage* srcImg){
    IplImage *srcImg;
    CopyMat2Ipl(srcImgMat,&srcImg);
    IplImage *skinGrayImg = NULL;				//灰階的〔膚色影像〕

    //〔膚色偵測〕：取得手部區域
    //ＰＳ：若在判斷過程有偵測到〔包夾區域〕，則回傳True
    bool isFind = getSkinArea(srcImg,&skinGrayImg);
    //辨識手上〔握物物品〕
    isFind = (isFind)? isFind : findObject(srcImg,skinGrayImg, storage);
    /*****************************************************
    *					釋放記憶體處理程序
    *****************************************************/
    cvReleaseImage(&srcImg);
    //cvReleaseImage(&destImg);
    cvReleaseImage(&skinGrayImg);
    cvClearMemStorage(storage);
    cellDetectLog.printLog("check thread cellDetect() exit");
    return isFind;
}

/*********************************************************
*〔Function〕取得膚色連通區塊
*〔參數〕
*	srcImg			:	原始影像（彩色）
*	skinGrayImg		:	膚色區塊的影像（灰階）
*
**********************************************************/
bool findObject(IplImage* srcImg,IplImage *skinGrayImg, CvMemStorage* storage) {
    cellDetectLog.printLog("check thread findObject() enter");
    //原始影像大小
    CvSize imageSize = cvSize(srcImg->width, srcImg->height);

    //存放分析結果的影像
    IplImage* resultImg = cvCreateImage(imageSize,IPL_DEPTH_8U,srcImg->nChannels);
    cvCopyImage(srcImg,resultImg);


    CvSeq* contourSeq;											//擷取〔手部輪廓〕的存放容器
    IplImage* tempImg = cvCloneImage(skinGrayImg);				//處理過程的暫存影像[載入膚色的區塊]


    //先過濾掉整張影像中輪廓的雜訊
    cvSetImageROI(tempImg, cvRect(0, 0, imageSize.width, imageSize.height));
    clearImgNoise(tempImg,tempImg);

    // 创建一个空序列用于存储轮廓角点
    CvSeq* squares = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), storage);

    /************************************************************
    *	接著,再以膚色區塊做為遮罩,與原始影像（彩色）做ＡＮＤ運算
    *************************************************************/
    {
        IplImage *skinByAndImg = cvCreateImage(imageSize, IPL_DEPTH_8U, 3);
        cvSmooth(tempImg,tempImg,CV_MEDIAN,3,3,0);			//先用〔中值濾波〕過濾掉雜訊
        colorAND(srcImg,tempImg,skinByAndImg);				//再進行ＡＮＤ運算（膚色的取交集）

        debugShowImage("膚色取AND",skinByAndImg);
        cvReleaseImage(&skinByAndImg);
    }

    IplImage* skinContourImg = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);			//

    //先用Ｃａｎｎｙ演算法進行〔輪廓偵測〕
    cvCanny(tempImg, skinContourImg, 150, 200, 3);
    cvReleaseImage(&tempImg);

    cvDilate(skinContourImg, skinContourImg, 0, 1);
    debugShowImage("膚色輪廓",skinContourImg);


    //接著再去〔抓取輪廓〕
    int tourCount = cvFindContours(
            skinContourImg,					//二值化的輸入影像
            storage,						//記憶體容器
            &contourSeq,					//輸出輪廓的容器（雙重指標）
            sizeof(CvContour),				//每個輪廓的記憶體大小
            CV_RETR_EXTERNAL,				//輪廓〔擷取模式〕：
                                            // CV_RETR_LIST		－ 抓取所有輪廓，並且只存放在第一層的Ｌｉｓｔ中（i.e. 沒有 Child）
                                            // CV_RETR_EXTERNAL － 抓取最外層的輪廓
            CV_CHAIN_APPROX_NONE,
            //CV_CHAIN_APPROX_SIMPLE,		//逼近方法：
                                            //	CV_CHAIN_APPROX_NONE	－	抓取所有輪廓上的點
                                            //	CV_CHAIN_APPROX_SIMPLE	－	只保留末端的 Pixel

            cvPoint(0, 0)					//每個〔輪廓點〕的偏移量
        );

    cvReleaseImage(&skinContourImg);

    //前兩大輪廓的暫存器
    double maxTourArea1 = -1;
    double maxTourArea2 = -1;
    CvSeq *maxTour1 = NULL;
    CvSeq *maxTour2 = NULL;

    /************************************************************************/
    /* 檢查所有輪廓：只會去保留前兩個面積最大的輪廓
    /************************************************************************/
    for (; contourSeq != NULL; contourSeq = contourSeq->h_next) {
        //計算輪廓面積
        double dConArea = fabs(cvContourArea(contourSeq, CV_WHOLE_SEQ));

        //該輪廓面積必須大於基本門檻
        if(dConArea > 200){

            //紀錄最大的輪廓
            if(maxTourArea1 < dConArea){
                if(maxTourArea1 != -1 && maxTourArea2 == -1){
                    maxTourArea2 = maxTourArea1;
                    maxTour2 = maxTour1;
                }

                maxTourArea1 = dConArea;
                maxTour1 = contourSeq;

            //以及第二大的輪廓
            } else if(maxTourArea2 < dConArea){
                maxTourArea2 = dConArea;
                maxTour2 = contourSeq;
            }
        }
    }

    //若沒有出現任何〔膚色區域〕,就代表沒出現使用手機的特徵
    if(maxTour1 == NULL){
        cvReleaseImage(&resultImg);
        cellDetectLog.printLog("check thread findObject() exit1");
        return false;
    }

    vector <HandArea*> handArea;			//記錄前兩大可能為〔手部區域〕的物件

    //取得〔前兩大〕面積為最大的膚色區域,當作是手部的位置
    find_2MaxArea(handArea,maxTour1,maxTour2);

//	return false;
    //測試:之後要將 3 -> 1
    IplImage *showImg = cvCreateImage(imageSize,skinGrayImg->depth,3);					//用來顯示輪廓資訊的影像 (灰階 - 測試用途)
    IplImage *handContourMaskImg = cvCreateImage(imageSize,skinGrayImg->depth,1);		//手部區域的遮罩影像 (灰階)
    IplImage *holdMaskImg = cvCreateImage(imageSize,skinGrayImg->depth,1);				//要存放手部(輪廓外圍)握有東西的遮罩 (灰階)

    cvZero(handContourMaskImg);
    cvZero(showImg);

    /****************************************************
    *				繪製手部物件的資訊
    ****************************************************/
    for (int i = 0;i < handArea.size(); i++) {
        //繪製輪廓資訊
        cvDrawContours(showImg, handArea[i]->contour, CV_RGB(255>>1, 255>>1, 255>>1), CV_RGB(0, 0,0),1, 1 );
        cvDrawContours(handContourMaskImg, handArea[i]->contour, CV_RGB(255, 255, 255), CV_RGB(255, 255,255),1,CV_FILLED);
        //drawRectangle(showImg,handArea[i]->rectArea,getRandColor(),3);		//整個手部的方型區域
        //cvDrawCircle(showImg,handArea[i]->center,2,getRandColor(),3);			//手部質心點

        //取的手部輪廓的〔近似曲線〕,以致節點個數可以做化簡
        handArea[i]->approContour =
                            cvApproxPoly(
                                handArea[i]->contour,								//邊緣的點集合序列
                                sizeof(CvContour),									//逼近曲線尺寸
                                storage,											//逼近輸入的曲線容器
                                CV_POLY_APPROX_DP,									//使用 Douglas-Peucke 演算法
                                cvContourPerimeter(handArea[i]->contour) * 0.02,	//逼近曲線的精度
                                0													//若 parameter2 = 0(非封閉曲線) : != 0 (封閉曲線)
                            );

        //繪製找到的近似曲線
        cvDrawContours(showImg, handArea[i]->approContour, CV_RGB(255, 255, 255), CV_RGB(0, 0,0),1, 1 );

        //找尋手上「可能握有手機的區域」
        bool flag = findHoldEdge(*handArea[i]);

        debugPrintf("找尋結果 = %s (%d~%d)\n\n",
                    handArea[i]->isFindHoldArea?"找到":"找不到",
                    handArea[i]->holdLeftIndex,
                    handArea[i]->holdRightIndex
                );
    }
/*

    if( ( handArea.size() == 1 && !handArea[0]->isFindHoldArea) ||
        ( handArea.size() == 2 && !handArea[0]->isFindHoldArea && !handArea[1]->isFindHoldArea)
    ) return false;*/


    //再去檢查〔雙手的情形〕，以致分析出來的結果能夠更精確
    doubleHandCheck(handArea);

    //以及〔單手握物〕可能的情形
    singleHandCheck(handArea);


    /************************************************************************/
    /*					顯示手部輪廓〔分析結果〕的資訊
    /************************************************************************/
    CvFont font = cvFont(1,1);		//要在影像上繪製文字的大小
    //
    for(int i = 0;false && i < handArea.size();i++){
        //對每個〔手部關節點〕做處理
        for (int j = 0,n = handArea[i]->joints.size(); j < n; j++) {
//            printf("[%d] 座標 = (%d,%d)\t角度 = %0.lf (%s : %s)\n",
//                        j,
//                        handArea[i]->joints[j].point.x,						//座標Ｘ
//                        handArea[i]->joints[j].point.y,						//座標Ｙ
//                        handArea[i]->joints[j].angle,						//輪廓的夾角
//                        (handArea[i]->joints[j].isOutside)?	"外":"內", 		//是否為外角
//                        (handArea[i]->joints[j].isHold)?	"有":"無"		//是否為握有東西的區域
//                );

            char text[100];
            sprintf(text,"%d",j);

            //繪製握物區域
            if(	handArea[i]->joints[j].isHold &&
                handArea[i]->joints[ (j+1) % n ].isHold)
            {
                cvLine(	showImg,
                        cvPoint(handArea[i]->joints[j].point.x,handArea[i]->joints[j].point.y),
                        cvPoint(handArea[i]->joints[(j+1) % n].point.x,handArea[i]->joints[(j+1) % n].point.y),
                        CV_RGB(255,0,0),5 );
            }

            //在影像上〔繪製文字〕:關節點編號
            cvPutText(	showImg,
                        text,
                        cvPoint(handArea[i]->joints[j].point.x - 10	,
                                handArea[i]->joints[j].point.y		),
                        &font,
                        CV_RGB(255,255,0)
                    );
        }
    }

    debugShowImage("手部區域",handContourMaskImg);
    debugShowImage("輪廓分析結果",showImg);
    cvReleaseImage(&showImg);
    //return false;

    //取得手部可能握有東西的 [遮罩]
    if(getHoldMask(handContourMaskImg,holdMaskImg,handArea) == false ) {
        //printf("不存在握物區域!!\n");
        cvReleaseImage(&handContourMaskImg);
        cvReleaseImage(&holdMaskImg);
        cvReleaseImage(&resultImg);
        cellDetectLog.printLog("check thread findObject() exit2");
        return false;
    }

    IplImage *holdImg = cvCreateImage(imageSize,srcImg->depth,srcImg->nChannels);		//用來顯示手上握什麼東西的影像 (彩色)

    //最後,再利用GrabCut演算法找出手上拿的物品
    grabCutProcess(srcImg,&holdImg,holdMaskImg,cvRect(0,0,srcImg->width,srcImg->height));
    debugShowImage("手上的物體",holdImg);

    //之後再去利用偵測到的物件，判斷是否為〔方型物體〕
    bool isMatch = cellAnalysis(srcImg,resultImg,holdImg,handContourMaskImg);

    //if(isMatch)		cvShowImage("偵測結果",resultImg);

    cvReleaseImage(&holdImg);
    cvReleaseImage(&holdMaskImg);
    cvReleaseImage(&handContourMaskImg);
    cvReleaseImage(&resultImg);
    cellDetectLog.printLog("check thread findObject() exit3");
    return isMatch;
}

/************************************************************************/
/*[Function] 物體分析：判斷偵測出來的物品是否為方型的物件
/*[參數]
/*	srcImg		:	原始影像 (彩色)
/*	holdImg		:	手部〔拿物體〕的影像 (彩色)
/*	handMaskImg	:	〔手部區域〕的輪廓 (灰階)
/*
/************************************************************************/
bool cellAnalysis(IplImage *srcImg,IplImage *resultImg,IplImage *holdImg,IplImage *handMaskImg){
    cellDetectLog.printLog("check thread cellAnalysis() enter");
    const static double MATCH_THRESH_RATIO = 0.01;

    CvSize imageSize = cvGetSize(srcImg);						//影像大小
    CvSeq *holdContour;											//手機的輪廓
    double matchRatio;											//〔相似度〕匹配的結果
    double cellW,cellH;											//物品的〔高度〕與〔寬度〕

    IplImage *holdBinImg = cvCreateImage(imageSize,8,1);		//物體的二值化影像 (灰階)
    IplImage *holdTourImg = cvCreateImage(imageSize,8,1);		//物品輪廓的影像   (灰階)


    /************************************************************************/
    /* 先針對偵測到的物品做做二值化處理：
    /* 屬於外圍區域就設為〔背景〕（黑），屬於物品區域就設為〔前景〕（白）
    /************************************************************************/
    bgThreshold(holdImg,holdBinImg,Scalar(128,128,128) );
    debugShowImage("物體二值化",holdBinImg);

    //接著再去做〔雜訊處理〕
    clearImgNoise(holdBinImg,holdBinImg);
    cvDilate(holdBinImg,holdBinImg,0,1);

    //之後再去針測手上物品的輪廓
    cvCanny(holdBinImg,holdTourImg,200,300,3);
    debugShowImage("物體輪廓",holdTourImg);

    //接著再去做輪廓處理
    cvFindContours(		holdTourImg,					//二值化的輸入影像
                        storage,						//記憶體容器
                        &holdContour,					//輸出輪廓的容器（雙重指標）
                        sizeof(CvContour),				//每個輪廓的記憶體大小
                        CV_RETR_EXTERNAL,				//輪廓〔擷取模式〕
                        CV_CHAIN_APPROX_NONE,			//逼近方法
                        cvPoint(0, 0)					//每個〔輪廓點〕的偏移量
                    );

    cvReleaseImage(&holdTourImg);

    /************************************************************************/
    /* 找尋最大周長的輪廓
    /************************************************************************/
    double maxContourLen = -1;
    int maxContourIndex;
    CvSeq *maxTourPtr = NULL;

    for (int i = 0; holdContour != NULL; holdContour = holdContour->h_next,i++) {
        //計算輪廓周長
        double dConArea = fabs(cvArcLength(holdContour, CV_WHOLE_SEQ));

        //紀錄最大周長的輪廓
        if(dConArea > maxContourLen){
            maxContourLen = dConArea;
            maxTourPtr = holdContour;
            maxContourIndex = i;
        }
    }

    if(maxTourPtr == NULL){
        cvReleaseImage(&holdBinImg);
        cellDetectLog.printLog("check thread cellAnalysis() exit1");
        return false;
    }

    /************************************************************************/
    /* 建立該物品輪廓的〔最小矩形〕的遮罩
    /************************************************************************/
    //找出包含輪廓的[最小矩形]
    CvBox2D minRect = cvMinAreaRect2(maxTourPtr);

    //各別紀錄矩形的〔寬度〕與〔高度〕	（ＰＳ：寬度　＞＝　高度）
    cellH = minRect.size.height;	//(minRect.size.width < minRect.size.height)?	minRect.size.width : minRect.size.height;
    cellW = minRect.size.width;		//(minRect.size.width >= minRect.size.height)?	minRect.size.width : minRect.size.height;

    /************************************************************************/
    /* 手機方型〔長/寬〕與〔面積〕比例檢查
    /************************************************************************/
    if(	(cellH - imageSize.height) >=  ((int)imageSize.height >> 4)		||	//手機〔方型高度〕必須不可超過〔畫面高度〕的 15/16
        (cellW - imageSize.width)  >=  ((int)imageSize.width >> 4)		||	//手機〔方型寬度〕必須不可超過〔畫面寬度〕的 15/16
        cellH * cellW >= (int)(imageSize.height*imageSize.width) >> 1		//手機〔方型面積〕也必須低於〔畫面面積〕的 1/2
    ){
        debugPrintf("比例錯誤!\n");
        cvReleaseImage(&holdBinImg);
        cellDetectLog.printLog("check thread cellAnalysis() exit2");
        return false;
    }

    IplImage *rectMaskImg = cvCreateImage(imageSize,8,1);		//物體的最小〔矩形遮罩〕
    cvZero(rectMaskImg);

    int numRectPoint = 4;
    CvPoint2D32f rectPoints_32f[4];			//[旋轉矩形] 結構上的頂點
    CvPoint rectPoints[4];					//[正規矩形] 結構上的頂點
    CvPoint *rectPointPtr = rectPoints;

    {	//找出矩形的四個 [頂點]
        cvBoxPoints(minRect, rectPoints_32f);

        //將 CvPoint2D32f 型態專換成 CvPoint
        for(int i = 0; i < numRectPoint; i++)
        rectPoints[i] = cvPointFrom32f(rectPoints_32f[i]);

        //繪製該 〔方形遮罩〕（by 封閉多邊形）
        cvFillConvexPoly(rectMaskImg,rectPoints,4,Scalar(255),CV_AA);
    }

    //先用最大的輪廓區域做填補,避免輪廓內部可能出現小黑點
    cvDrawContours(holdBinImg,maxTourPtr,Scalar(255),Scalar(255),1,CV_FILLED);

    //接著只在該方型的區域範圍內做ＯＲ運算：使得〔物品區域〕與〔手部區域〕可以做結合
    cvOr(holdBinImg,handMaskImg,holdBinImg,rectMaskImg);

    //再去將可能存在的小黑點做填補
    cvDilate(holdBinImg,holdBinImg,0,1);

    //並且將一些可能出現灰色的〔過度地帶〕變成前景（白色）的一部分
    cvThreshold(holdBinImg,holdBinImg,0.1,255,CV_THRESH_BINARY);

    //之後再以方型的區域做裁切，避免產生外部區域不必要的雜點
    cvAnd(holdBinImg,rectMaskImg,holdBinImg);

    //最後，再去用〔模板匹配〕的方式：去比較偵測到的〔方形區域〕與〔物品＋手部〕結合的區域
    matchRatio = cvMatchShapes(rectMaskImg,holdBinImg,3);

   // printf("相似度：%lf\n",matchRatio);
   // printf("寬度：%lf\t高度：%lf\t比例：%lf\n",cellW,cellH, cellW/cellH);

    //判斷兩張輪廓的〔相似度〕是否有在門檻值的範圍內
    if(matchRatio <= MATCH_THRESH_RATIO) {
        //若相似度在門檻範圍內,則將該物品的區域用矩形框出來
        cvPolyLine(resultImg, &rectPointPtr, &numRectPoint, 1, 1, CV_RGB(255,0,0), 2);
    }

    debugShowImage("方形遮罩",rectMaskImg);
    debugShowImage("填補手機輪廓",holdBinImg);

    cvReleaseImage(&rectMaskImg);
    cvReleaseImage(&holdBinImg);

    //若相似度有在門檻範圍內,則回傳True;否則,回傳False
    cellDetectLog.printLog("check thread cellAnalysis() exit3");
    return (matchRatio <= MATCH_THRESH_RATIO);
}

/*********************************************************
*〔Function〕進行ＧｒａｂＣｕｔ演算法的計算
*〔參數〕
*	srcImg			:	原始影像（彩色）
*	destImg			:	要存放分析結果的影像（彩色）
*	maskImg			:	GrabCut專屬的遮罩（灰階，數值＝０～３）
*	ROI				:	要處理的區域
*
**********************************************************/
void grabCutProcess(IplImage *srcImg,IplImage **destImg,IplImage *maskImg,CvRect ROI){
    cellDetectLog.printLog("check thread grabCutProcess() enter");
    /************************************************************************/
    /* 進行ＧｒａｂＣｕｔ演算法的處理程序
    /************************************************************************/
    cv::Mat bgModel,fgModel;							//GrabCut演算法內部使用的容器

    cv::Mat srcMat = Ipl2Mat(srcImg).clone();			//先將原始影像
    cv::Mat maskMat = Ipl2Mat(maskImg).clone();			//與遮罩影像	轉換成Ｍａｔ空間

    //cv::imshow("使用遮罩",maskMat * 85);
    //cvWaitKey(0);

    cv::grabCut(	srcMat,								//原始影像 (彩色)
                    maskMat,							//要輸入的遮罩(僅限於ＭＡＳＫ模式底下有效)；處理後的結果也將存放在該容器中
                    ROI,								//處理範圍ＲＯＩ	 (僅限於ＲＥＣＴ模式底下有效)
                    bgModel,							//內部演算法所使用的〔背景模型〕（13 x 5）
                    fgModel,							//內部演算法所使用的〔前景模型〕（13 x 5）
                    1,									//要做處理的 Recursion 次數
                    cv::GC_INIT_WITH_MASK				//處理模式：
                                                        //		GC_INIT_WITH_MASK	－	遮罩模式
                                                        //		GC_INIT_WITH_RECT	－	區域模式
                );

    //將〔確定前景〕與〔預期前景〕的Ｐｉｘｅｌ做合併
    maskMat = maskMat & 1 ;

    cv::Mat foregroundMat(srcMat.size() , CV_8UC3 ,  cv::Scalar(128,128,128)	) ;		//前景影像

    //將〔原始影像〕與〔遮罩影像〕整合成〔前景影像〕
    srcMat.copyTo(foregroundMat , maskMat);

    //最後,再將影像複製到目標影像中
    CopyMat2Ipl(foregroundMat,destImg);
    //cvCopy(&IplImage(foregroundMat),destImg);

    //釋放記憶體
    //cvReleaseImage(&maskImg);
    bgModel.release();
    fgModel.release();
    srcMat.release();
    maskMat.release();
    foregroundMat.release();
    cellDetectLog.printLog("check thread grabCutProcess() exit");
}

/*********************************************************
*〔Function〕計算ＧｒａｂＣｕｔ演算法的遮罩
*〔參數〕
*	srcImg			:	原始影像（彩色）
*	maskImg			:	處理後的遮罩影像（灰階）
*	hand			:	存放〔輪廓〕與〔質心〕關係的物件
*
**********************************************************/
void getGrabCutMask(IplImage *srcImg,IplImage *maskImg,HandArea *hand){
    cellDetectLog.printLog("check thread getGrabCutMask() enter");
    //設定ＲＯＩ外部的區塊為〔背景〕
    cvZero(maskImg);

    //設定ＲＯＩ內部區塊為〔預期背景〕
    drawRectangle(maskImg,hand->rectArea,Scalar(cv::GC_PR_BGD),CV_FILLED);

    //設定手部輪廓區塊為〔預期前景〕
    cvDrawContours(maskImg,hand->contour,Scalar(cv::GC_PR_FGD),Scalar(cv::GC_PR_FGD),1,CV_FILLED);

    //判斷〔質心〕座標與〔手部輪廓〕的相對應關係，來決定〔質心圓〕的遮罩為何
    double threahDist = 10.0;			//〔質心〕到〔手部輪廓〕的門檻值
    bool isNearContour = (fabs(hand->isInDistance) < threahDist);		//「質心到輪廓」的最小距離是否小於門檻值

    //最低位元（若質心是在輪廓內部，就設定為前景；反之，設定為背景）
    int LSB			= (hand->isInDistance > 0)?		1	:	0	;

    //若質心到輪廓的距離小於門檻值，則設定為〔預設〕狀態（offset = 1）；反之，可以確認是輪廓中的一部分
    int offsetBit	= (isNearContour == true)?		1	:	0	;

    //計算出最終的〔遮罩位元〕
    int finalBit	= ( (1 & offsetBit) << offsetBit) + LSB;

    //計算質心圓的半徑：若距離小於門檻值,則半徑設定為２；否則,以半徑除以2做為質心圓的半徑
    int radius = (isNearContour == true)?	2	:	( (int) fabs(hand->isInDistance) >> 1);
    //printf("R = %d [%d = %d,%d]\n",radius,finalBit,LSB,offsetBit);

    cvDrawCircle(maskImg,hand->center,radius,cvScalar(finalBit),CV_FILLED);
    cellDetectLog.printLog("check thread getGrabCutMask() exit");
}

/*********************************************************
*〔Function〕找尋最大的兩個輪廓,並且加入到vector中
*〔參數〕
*	handArea		:	要存放手部物件的容器
*	tour1			:	輪廓１
*	tour2			:	輪廓２
*
**********************************************************/
void find_2MaxArea(vector <HandArea*> &handArea,CvSeq *tour1,CvSeq *tour2){
    cellDetectLog.printLog("check thread find_2MaxArea() enter");
    //若兩個輪廓指標指向同一個位置,則判定為僅有一個最大輪廓；否則，就存在兩個輪廓
    int handAreaCount = (tour2 == NULL)?		1	:	2;
    handArea.reserve(handAreaCount);
    handArea.push_back(new HandArea(tour1) );		//加入輪廓１

    /************************************************************************/
    /* 檢查第二大的輪廓是否該加進來
    /************************************************************************/
    if(handAreaCount == 2){
        double area1 = fabs(cvContourArea(tour1, CV_WHOLE_SEQ));
        double area2 = fabs(cvContourArea(tour2, CV_WHOLE_SEQ));

        //第一個輪廓 與 第二個輪廓的面積差距不可以太大
        if((area1 - area2)/area1 > 0.5 ){
            handAreaCount = 1;

        //否則,面積差距不大就將第二個輪廓加進來
        } else {
            handArea.push_back(new HandArea(tour2) );
        }
    }

    //printf("[%d] Max = %lf %lf,Size = %d\n",handAreaCount,maxTourArea1,maxTourArea2);

    for (int i = 0;i < (int) handArea.size();++i) {

        {	//最小方形區塊計算
            CvRect rect = cvBoundingRect(handArea[i]->contour,0);
            handArea[i]->rectArea = rect;
        }

        {	//輪廓質心計算
            CvPoint center;
            getCenter(handArea[i]->contour,center);
            handArea[i]->center = center;

            //並且計算質心到輪廓的最小距離,以便將來遮罩的處理判斷
            handArea[i]->isInDistance = cvPointPolygonTest(handArea[i]->contour,Point2Point32f(center),true);
        }
    }
    cellDetectLog.printLog("check thread find_2MaxArea() exit");
}

/************************************************************************/
/*[Function] 檢查手部內部包夾區域
/*〔參數〕
/*	hankSkinImg		:	手部膚色遮罩區域（二值圖）
/*
/************************************************************************/
bool checkHandWrap(IplImage *hankSkinImg){
    cellDetectLog.printLog("check thread checkHandWrap() enter");
    bool isFind = false;													//紀錄是否有找到包夾區塊
    CvSize imageSize = cvSize(hankSkinImg->width, hankSkinImg->height);		//影像大小
    IplImage* skinContourImg = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);	//處理〔手部輪廓〕的影像

    {	//先用Ｃａｎｎｙ演算法進行〔輪廓偵測〕
        cvCanny(hankSkinImg, skinContourImg, 150, 200, 3);
        cvDilate(skinContourImg, skinContourImg, 0, 1);
    }

    /************************************************************************/
    /*						先擷取手部輪廓
    /************************************************************************/
    CvSeq *handContour = NULL;			//存放〔手部輪廓〕的容器

    cvFindContours(
        skinContourImg,					//二值化的輸入影像
        storage,						//記憶體容器
        &handContour,					//輸出輪廓的容器（雙重指標）
        sizeof(CvContour),				//每個輪廓的記憶體大小
        CV_RETR_CCOMP,					//輪廓〔擷取模式〕：內∕外兩層式的擷取模式
        CV_CHAIN_APPROX_NONE,			//逼近方法：抓取所有輪廓上的點
        cvPoint(0, 0)					//每個〔輪廓點〕的偏移量
    );

    //記錄目前符合條件的〔內部面積最大輪廓〕
    double maxArea = -FLT_MAX;
    CvSeq *maxContour = NULL;

    //面積門檻必須大於整張影像的 1/32
    double areaThreshold = (imageSize.width * imageSize.height) >> 5;

    /************************************************************************/
    /* 檢查所有〔外層輪廓〕
    /************************************************************************/
    for (CvSeq *outerContour = handContour; outerContour != NULL; outerContour = outerContour->h_next) {

        /************************************************************************/
        /* 檢查所有〔內層輪廓〕
        /************************************************************************/
        for(CvSeq *innerContour = outerContour->v_next;innerContour != NULL;innerContour = innerContour->h_next){
            double innerArea = fabs(cvContourArea(innerContour, CV_WHOLE_SEQ));		//內部輪廓面積

            //符合條件時,才去更新目前〔內部最大輪廓〕
            if(	innerArea >= areaThreshold &&			//〔內部輪廓面積〕必須大於門檻值
                innerArea > maxArea						//且大於〔之前記錄〕時才做更新
            ){
                maxArea = innerArea;
                maxContour = innerContour;
            }
        }
    }

    //若沒有一個符合條件的輪廓，就不繼續做判斷
    if(maxContour == NULL)	goto noFindLabel;

    {	/************************************************************************/
        /*	直接判斷〔中心點〕是否為〔膚色區域〕（白色）的一部分
        /************************************************************************/
        //先取得質心
        CvPoint center;
        getCenter(maxContour,center);

        //再判斷質心點的〔顏色強度〕是否為〔白色區域〕的一部分
        CvScalar centerScalar = cvGet2D(hankSkinImg, center.y, center.x);
        //printf("中心點 = %.2f\n",centerScalar.val[0]);

        if(centerScalar.val[0] > 0.1f)	goto noFindLabel;
    }

    /************************************************************************/
    /* 最後，再去檢查
    /*	〔包夾面積〕必須低於〔畫面大小〕的１／４
    /************************************************************************/
    isFind =	(maxArea <= (imageSize.width * imageSize.height) << 2 );

    if(isFind){
        /*		在〔握物區域〕繪製紅色框框		*=/
        CvRect rect = cvBoundingRect(maxContour,0);
        cvRectangle(	hankSkinImg,
                        cvPoint(rect.x, rect.y),
                        cvPoint(rect.x + rect.width, rect.y + rect.height),
                        CV_RGB(255, 0, 0), 2, 8, 0);
        /*										*/

        //繪製包夾的輪廓
        for(int i = 0,lengthContour = maxContour->total;
            i < lengthContour;
            i++
        ){
                CvPoint point =	*(CvPoint*) cvGetSeqElem (maxContour, i);
                cvDrawCircle(hankSkinImg, point, 3, Scalar(0,0,255), CV_FILLED);
        }

       // printf(	"內部面積 = %.2lf\n",maxArea);
        //cvShowImage("包夾輪廓",hankSkinImg);
    }

noFindLabel:

    cvReleaseImage(&skinContourImg);
    cellDetectLog.printLog("check thread checkHandWrap() exit");
    return isFind;
}
