#include "stdafx.h"
#include "header.h"
#include "outputLog.h"

outputLog skindetecterLog("skindetecter.log");
/*********************************************************
*〔Function〕取得膚色連通區塊，並且判斷有沒有出現〔包夾區域〕
*〔參數〕
*	srcImg			:	原始影像（彩色）
*	skinGrayImg		:	膚色區塊的影像（灰階）
*
**********************************************************/
bool getSkinArea(IplImage *srcImg,IplImage **skinGrayImg){
    skindetecterLog.printLog("check thread getSkinArea() enter");
    IplImage* tempImg = cvCloneImage(srcImg);		//〔處理過程〕的暫存影像
    IplImage *dilateImg = NULL;						//〔膚色膨脹〕後影像

    //〔雜訊處理〕程序
    clearImgNoise(srcImg,tempImg);

    //RGB -> YCrCb
    cvCvtColor(tempImg,tempImg,CV_BGR2YCrCb);
    skinColorDetection(tempImg,tempImg);					//膚色二值化


    {	/************************************************************************/
        /*						檢查手部〔包夾物〕件的情況
        /************************************************************************/
        dilateImg = cvCreateImage(cvSize(tempImg->width, tempImg->height), tempImg->depth, tempImg->nChannels);
        cvDilate(tempImg, dilateImg);
        debugShowImage("膚色膨脹後", dilateImg);

        //包夾判斷
        bool flag = checkHandWrap(dilateImg);

        cvReleaseImage(&tempImg);

        if(flag){
            cvReleaseImage(&dilateImg);
            skindetecterLog.printLog("check thread getSkinArea() exit1");
            return true;
        }
    }

    /************************************************************************/
    /*	連通區域處理程序：將膚色的小碎塊，組合成連通區域做分析
    /************************************************************************/
    cv::Mat skinBinImage = Ipl2Mat(dilateImg);								//〔膚色灰階化〕影像
    cv::Mat labelImg;														//〔連通區域〕影像

    cv::cvtColor(skinBinImage,skinBinImage,CV_RGB2GRAY);					//灰階化
    cv::threshold(skinBinImage, skinBinImage, 1, 1, CV_THRESH_BINARY) ;		//非０以上的標籤區塊的數值轉換成數值「１」
    //binImage *= 30;
    //cv::imshow("二值化", binImage);

    //利用 Two-Pass 演算法,將膚色的區塊組合成〔連通區域〕
    connectTwoPassAlgo(skinBinImage, labelImg);

    {	/************************************************************************/
        /*   顯示連通區域的彩色影像
        /************************************************************************/
        cv::Mat colorLabelImg;													//〔彩色化〕後的連通區域影像

        //接著再將這些連通區域用不同的〔色彩〕做標記
        connectShowColor(labelImg, colorLabelImg);

        IplImage *colorSkinImg; //= cvCloneImage( &IplImage(colorLabelImg) );
        CopyMat2Ipl(colorLabelImg,&colorSkinImg);
        debugShowImage("膚色連通區域", colorSkinImg);

        *skinGrayImg = cvCloneImage(colorSkinImg);
        *skinGrayImg = getGrayImg(*skinGrayImg);


        //記憶體釋放
        cvReleaseImage(&colorSkinImg);
        skinBinImage.release();
        colorLabelImg.release();
        labelImg.release();

    }

    cvThreshold(*skinGrayImg,*skinGrayImg,1,255,CV_THRESH_BINARY);
    debugShowImage("灰階聯通圖",*skinGrayImg);

    cvReleaseImage(&dilateImg);
    skindetecterLog.printLog("check thread getSkinArea() exit2");
    return false;
}

/*********************************************************
*〔Function〕膚色偵測
*〔參數〕
*	srcImg		:	要處理的原始影像（Ｙ Ｃｒ Ｃｂ色彩空間）
*	destImg		:	膚色區塊的影像（彩色－二值灰階結果）
*
**********************************************************/
void skinColorDetection(IplImage *srcImg,IplImage *destImg){
    skindetecterLog.printLog("check thread skinColorDetection() enter");
    double cb, cr,y;		//每個Pixel Ｙ、Ｃｒ、ＣＢ　的數值

    /**********************************************************
    *				對每個 Pixel做處理
    ***********************************************************/
    for( int i = 0; i < srcImg->height; i++ ){
        for( int j = 0; j < srcImg->width; j++ ){
            //先取得該Pixel座標的ＹＣｒＣｂ數值
            CvScalar scalarImg = cvGet2D(srcImg, i, j);
            y = scalarImg.val[0];
            cr = scalarImg.val[1];
            cb = scalarImg.val[2];

            //若ＹＣｒＣｂ顏色空間介於〔膚色範圍：Ｃｂ＝100～127;Ｃｒ＝138～170〕
            //則將目標影像設定為白色（前景）
            if(
                (100 <= cb && cb <= 127)	&&
                (138 <= cr && cr <= 170)
            ){
                cvSet2D(destImg, i, j, cvScalar( 255, 255, 255));

            //否則，設定為黑色（背景）
            } else {
                cvSet2D(destImg, i, j, cvScalar( 0, 0, 0));
            }
        }
    }
    skindetecterLog.printLog("check thread skinColorDetection() exit");
}

/*********************************************************
*〔Function〕連通元件合併－Ｔｗｏ　Ｐａｓｓ演算法
*〔參數〕
*	srcImg		:	要檢查連通元件的原始影像（灰階影像：０＝背景、１＝前景）
*	labelImg	:	要產生連通元件的目標影像（灰階影像）
*
*〔演算法〕
*	Ｓｔｅｐ１：【元件標記】
*				逐一由[左上角] 至 [右下角]檢察
*				若被標示為１則檢查４－鄰居的標籤（２～Ｎ）
*				若都沒有標記,則單獨產生一個新的標籤值
*				否則，曲鄰居中的〔最小標籤〕編號
*
*	Ｓｔｅｐ２：【標籤合併】
*				同樣由左上角至右下角逐一檢查
*				若鄰居中有出現標籤，則以鄰居中的〔最小標籤〕作取代
*
**********************************************************/
void connectTwoPassAlgo(const cv::Mat& srcImg, cv::Mat& lableImg){
    skindetecterLog.printLog("check thread connectTwoPassAlgo() enter");
    /****************************************
    *			檔案格式檢查程序
    ******************************************/
    if (srcImg.empty()				||
        srcImg.type() != CV_8UC1
    ){
        //printf("[Error] 輸入影像格式錯誤!\n");
         skindetecterLog.printLog("check thread connectTwoPassAlgo() exit1");
        return ;
    }

    /******************************************
    *〔Ｓｔｅｐ１〕元件標記
    *******************************************/
    //影像複製到要目標影像中
    lableImg.release() ;
    srcImg.convertTo(lableImg, CV_32SC1) ;


    int label = 1 ;					//目前的標籤編號	(起始值為2開始)
    std::vector<int> labelSet;
    labelSet.push_back(0) ;			//先放入〔背景〕	＝	０
    labelSet.push_back(1) ;			//與〔前景〕標籤	＝	１

    int rows = srcImg.rows ;
    int cols = srcImg.cols ;

    /*******************************************
    *	開始對每一個 Pixel做檢查
    ********************************************/
    for (int i = 1; i < rows - 1; ++i) {
        int* row1 = lableImg.ptr<int>(i-1) ;		//上列
        int* row2 = lableImg.ptr<int>(i) ;			//中列
        //int* row3 = lableImg.ptr<int>(i+1) ;		//下列

        for (int j = 1; j < cols - 1; ++j) {

            //只對目前紀錄為１（前景）的標籤做檢查
            if (row2[j] == 1) {
                std::vector<int> neighborLabels ;		//存放 [鄰居]的標籤紀錄
                neighborLabels.reserve(2) ;
                int upPixel = row1[j] ;					//上
                int leftPixel = row2[j - 1] ;			//左

                //開始檢查四周鄰居的標籤數值
                if (leftPixel	> 1)		neighborLabels.push_back(leftPixel) ;
                if (upPixel		> 1)		neighborLabels.push_back(upPixel) ;

                //若四周鄰居皆為空值(背景),則產生一個先的標籤做記錄
                if (neighborLabels.empty()){
                    labelSet.push_back(++label) ;
                    row2[j] = label ;
                    labelSet[label] = label ;

                //否則，配置四周鄰居中〔編號最小〕的標籤做記錄
                } else {
                    std::sort(neighborLabels.begin(), neighborLabels.end()) ;	//先做排序
                    int smallestLabel = neighborLabels[0] ;						//在挑選最小的編號
                    row2[j] = smallestLabel;									//進行標籤配置

                    //更新鄰居中〔同樣編號〕的標籤
                    for (size_t k = 1; k < neighborLabels.size(); k++){
                        int tempLabel = neighborLabels[k] ;
                        int &oldSmallestLabel = labelSet[tempLabel];

                        if (oldSmallestLabel > smallestLabel){
                            labelSet[oldSmallestLabel] = smallestLabel ;
                            oldSmallestLabel = smallestLabel ;

                        } else if (oldSmallestLabel < smallestLabel) {
                            labelSet[smallestLabel] = oldSmallestLabel ;
                        }
                    }
                }
            }
        }
    }

    /****************************************************
    * 對目前現有的〔所有標籤〕做更新：
    * 合併在同一個集合中的〔等價標籤〕，為編號最小的標籤
    *****************************************************/
    for (size_t i = 2; i < labelSet.size(); ++i) {
        int curLabel = labelSet[i] ;
        int preLabel = labelSet[curLabel] ;

        while (preLabel != curLabel){
            curLabel = preLabel ;
            preLabel = labelSet[preLabel] ;
        }

        labelSet[i] = curLabel ;
    }

    /*****************************************************
    *〔Ｓｔｅｐ２〕標籤合併：
    * 將在同一個等價標籤上的元素，合併成同一個標籤（元件）
    ******************************************************/
    for (int i = 0; i < rows; ++i) {
        int* data = lableImg.ptr<int>(i) ;

        for (int j = 0; j < cols; ++j) {
            int& pixelLabel = data[j] ;
            pixelLabel = labelSet[pixelLabel] ;
        }
    }

    /*****************************************************
    *〔Ｓｔｅｐ３〕元件篩選：
    * 淘汰標面積太小的元件
    ******************************************************/
    int *labelCount = (int * ) malloc(sizeof(int)* labelSet.size() );	//標籤編號 [次數統計]容器
    memset(labelCount,0,sizeof(int) * labelSet.size());

    std::map<int, int> labelMap;			//標籤編號 -> 加入該編號時的索引 (Index)
    std::map<int, int> labelMapInv;			//反向查詢：加入該編號時的索引 -> 標籤編號
    std::map<int, int> clearMap;			//記錄哪些元件要捨棄

    //先加入背景標籤(0)
    labelMap[0] = 0;
    labelMapInv[0] = 0;

    /******************************************
    *			對所有標籤進行統計
    ********************************************/
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int *label = lableImg.ptr<int>(i,j);

            //只去記錄目前仍未加入到Vector中的〔標籤編號〕
            if(*label > 0 && labelMap[*label] == 0){
                int index = labelMap.size();
                labelMap[*label] = index;
                labelMapInv[index] = *label;
            }

            //標籤編號統計
            ++labelCount[ labelMap[*label] ];
        }
    }

    /******************************************
    *		檢查哪一些標籤要被過濾
    *******************************************/
    int area = (srcImg.rows * srcImg.cols);
    int areaThread = (area > 25000)? area >> 4	:	200;

    for (int i = 0;i < labelMap.size();++i){

        //過濾掉太小的元件
        if(labelCount[i] < areaThread)		clearMap[ labelMapInv[i] ] = 1;
    }

    /****************************************************
    *	將所有被列入到清除清單中的元素設定為０（背景）
    ****************************************************/
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int *label = lableImg.ptr<int>(i,j) ;

            //若該Pixel的編號有出現在〔清除清單〕中,則將該元素的值設定為0
            if(	*label > 0 &&						//不對背景元素做檢察
                clearMap.count(*label) == 1	)		//有存在在清單中就做清空
            {
                *label = 0;
            }
        }
    }
     skindetecterLog.printLog("check thread connectTwoPassAlgo() exit2");
}

/*********************************************************
*〔Function〕對每個連通元件標示不同顏色,以識別不同的區塊
*〔參數〕
*	labelImg		:	已做完連通元件處理的灰階影像
*	colorLabelImg	:	要顯示連通元件區塊顏色的輸出影像
**********************************************************/
void connectShowColor(const cv::Mat& labelImg, cv::Mat& colorLabelImg)  {
     skindetecterLog.printLog("check thread connectShowColor() enter");
    //格式檢查
    if (labelImg.empty() ||
        labelImg.type() != CV_32SC1)
    {
        skindetecterLog.printLog("check thread connectShowColor() exit1");
        return ;
    }

    //紀錄〔標籤編號〕與〔顏色〕的對應表格
    std::map <int, cv::Scalar> colors ;

    int rows = labelImg.rows ;
    int cols = labelImg.cols ;

    //目標影像為彩色影像,但背景為黑色
    colorLabelImg.release() ;
    colorLabelImg.create(rows, cols, CV_8UC3) ;
    colorLabelImg = cv::Scalar::all(0) ;

    for (int i = 0; i < rows; ++i) {
        const int* data_src = (int*)labelImg.ptr<int>(i) ;
        uchar* data_dst = colorLabelImg.ptr<uchar>(i) ;

        for (int j = 0; j < cols; ++j) {
            int pixelValue = data_src[j] ;

            //只對是〔連通元件〕的 Pixel 做處理
            if (pixelValue > 1){

                //若該標籤仍未產生顏色，則隨機產生一個顏色做對應
                if (colors.count(pixelValue) <= 0) {
                    colors[pixelValue] = getRandColor() ;
                }

                //對該Pixel的座標上，做上色的操作
                cv::Scalar color = colors[pixelValue] ;
                *data_dst++= color[0] ;
                *data_dst++ = color[1] ;
                *data_dst++ = color[2] ;

            //否則，不做任何操作
            } else {
                ++data_dst;
                ++data_dst;
                ++data_dst;
            }
        }
    }
    skindetecterLog.printLog("check thread connectShowColor() exit2");
}
