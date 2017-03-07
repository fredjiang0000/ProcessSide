#include "QDebug"
#include "cellphone_detection.h"
#include "header.h"
#include "check_thread.h"
#include "outputLog.h"
/**************************************
 *              建構子
**************************************/
String handdetection_name = "handdetection.xml";
CascadeClassifier handdetection;
cellphone_detection::cellphone_detection(Mat* frame, int* detect_phone_sensitivity){
    this->myLog = new outputLog("cellphone_detection.log");
    myLog->printLog(logAddMem("cellphone_detection::cellphone_detection() enter"));
    this->frame = frame;
    this->detect_phone_sensitivity = detect_phone_sensitivity;
    this->doStop=false;
    myLog->printLog(logAddMem("cellphone_detection::cellphone_detection() exit"));

}
cellphone_detection::~cellphone_detection()
{
    myLog->printLog(logAddMem("cellphone_detection::~cellphone_detection()"));
    delete this->myLog;
    myLog = NULL;
}

void cellphone_detection::stop()
{
    myLog->printLog(logAddMem("cellphone_detection::stop() enter"));
    QMutexLocker locker(&doStopMutex);
    doStop=true;
    myLog->printLog(logAddMem("cellphone_detection::stop() exit"));
}
/************************************************
 * [function] : 標記存在背景的膚色物件，用於過濾背景膚色
 * frame : 背景frame
************************************************/
Vector<Rect> cellphone_detection::BG_Skin_mask(Mat frame){
    myLog->printLog(logAddMem("cellphone_detection::BG_Skin_mask() enter"));
    Vector<Rect> Output;
    Mat img_ROI;
    cvtColor(frame,img_ROI,CV_BGR2YCrCb);
    for(int i=0;i<img_ROI.rows;i++){
        for(int j=0;j<img_ROI.cols;j++){
            Vec3b Pixel =img_ROI.at<Vec3b>(i,j);
            if(Pixel[1] >= Cr_low && Pixel[1] <= Cr_up && Cb_low <= Pixel[2] && Pixel[2] <= Cb_up){
                img_ROI.at<Vec3b>(i,j)[0]=255;
                img_ROI.at<Vec3b>(i,j)[1]=255;
                img_ROI.at<Vec3b>(i,j)[2]=255;
            }
            else{
                img_ROI.at<Vec3b>(i,j)[0]=0;
                img_ROI.at<Vec3b>(i,j)[1]=0;
                img_ROI.at<Vec3b>(i,j)[2]=0;
            }

        }
    }

    cvtColor(img_ROI,img_ROI,CV_RGB2GRAY); //RGB->GRAY
    medianBlur(img_ROI,img_ROI,5);//中值濾波
    Mat element = getStructuringElement( 2, Size( 5,5), Point( 2,2 ) ); //宣告一元素，用於型態學處理
    vector < vector<Point2i > > blobs; // 紀錄連通圖區塊
    morphologyEx(img_ROI,img_ROI, MORPH_CLOSE, element , Point( -1,-1 ) , 10);//型態學-收縮 3次
    FindBlobs(img_ROI,blobs,0); // 連通圖Function
    for(size_t i=0; i < blobs.size(); i++){
        int sum_x=0,sum_y=0;
        int max_x=0,max_y=0;
        int min_x=10000,min_y=10000;
        for(size_t j=0; j < blobs[i].size(); j++) {
            int x = blobs[i][j].x;
            int y = blobs[i][j].y;
            if(x > 10 &&y >10){
                sum_x += blobs[i][j].x;
                sum_y += blobs[i][j].y;
            }
            if(max_x < x)
                max_x = x;
            if(max_y < y)
                max_y = y;
            if(min_x > x)
                min_x = x;
            if(min_y > y)
                min_y = y;

        }
        Rect temp_ROI;
        temp_ROI.x=min_x;
        temp_ROI.y=min_y;
        temp_ROI.width=max_x-min_x;
        temp_ROI.height=max_y-min_y;
        Output.push_back(temp_ROI);
    }
    myLog->printLog(logAddMem("cellphone_detection::BG_Skin_mask() exit"));
    return Output;
}


/*************************************
 *〔Function〕 Main 手機行為辨識
 ************************************/
void cellphone_detection::run(){
    myLog->printLog(logAddMem("cellphone_detection::run() enter"));
    if( !handdetection.load( handdetection_name ) )
    { printf("--(!)Error loading1\n"); myLog->printLog(logAddMem("cellphone_detection::run() exit1(Error loading)")); return;  };
    myLog->printLog(logAddMem("cellphone_detection::run() enter0-1"));
    objTrack objTracker = objTrack(this->myLog);
    myLog->printLog(logAddMem("cellphone_detection::run() enter0-2"));
    /***************skin BG**************/
    Mat bg_frame;
    Vector<Rect> skin_mask;
    /***********************************/
    namedWindow("catch",WINDOW_NORMAL);
//    namedWindow("BG SUB",WINDOW_NORMAL);
//    namedWindow("SkinMask",WINDOW_NORMAL);
    int counter=0;
    int updateNum = 0;
    int ROI_timer=0;
    Rect Human_ROI;
    vector<Mat> temp_Img;
    vector<Rect> temp_ROI;
    check_thread *CellPhone_thread  = new check_thread(); // 手機辨識thread
    CellPhone_thread->start(QThread::HighestPriority);
    //cout<< CellPhone_thread->currentThread() <<" CellPhone_thread start"<<endl;
    while(1){
        myLog->printLog(logAddMem("cellphone_detection::run() enter1"));
        ////////////////////////////////
        // Stop thread if doStop=TRUE //
        ////////////////////////////////
        doStopMutex.lock();
        if(doStop)
        {
            doStop=false;
            doStopMutex.unlock();
            break;
        }
        doStopMutex.unlock();
        /////////////////////////////////
        /////////////////////////////////
//        cout << "cellphone_detection detect_phone_sensitivity:" << *(this->detect_phone_sensitivity) << endl;
        /****************圖片錯誤判斷**********************************/
        if(frame->empty()){
            continue;
        }
        /*********************************************************/

        /********************skin BG**************/
        if(counter == 0 ){
            frame->copyTo(bg_frame);
            skin_mask = BG_Skin_mask(bg_frame);
        }

        /*****************************************/
        myLog->printLog(logAddMem("cellphone_detection::run() enter2"));
        Mat displayROI,track_frame;
        frame->copyTo(displayROI);
        frame->copyTo(track_frame);
        objTracker.tracker(track_frame);
        vector<Rect> obj = objTracker.getObj();

        myLog->printLog(logAddMem("cellphone_detection::run() enter3"));
        /*****************************************/
//        Mat skin;
//        cvtColor(displayROI,skin,CV_BGR2YCrCb);
//        for(int i=0;i<skin.rows;i++){
//            for(int j=0;j<skin.cols;j++){
//                Vec3b Pixel =skin.at<Vec3b>(i,j);
//                if(Pixel[1] >= Cr_low && Pixel[1] <= Cr_up && Cb_low <= Pixel[2] && Pixel[2] <= Cb_up){
//                    skin.at<Vec3b>(i,j)[0]=255;
//                    skin.at<Vec3b>(i,j)[1]=255;
//                    skin.at<Vec3b>(i,j)[2]=255;
//                }
//                else{
//                    skin.at<Vec3b>(i,j)[0]=0;
//                    skin.at<Vec3b>(i,j)[1]=0;
//                    skin.at<Vec3b>(i,j)[2]=0;
//                }

//            }
//        }
//        imshow("SkinMask",skin);
//        waitKey(1);
//        Mat BG_SUB;
//        BG_SUB = objTracker.binaryImage();
//        imshow("BG SUB",BG_SUB);
//        waitKey(1);
        /**********************呼叫 thread *********************************/
        /*****************  updateBG  *******************/
        if(obj.size()==0){
            updateNum++;
            if(updateNum>600){
                skin_mask = BG_Skin_mask(displayROI);
                updateNum=0;
            }
        }
        else if(obj.size()!=0) {
            updateNum=0;
        }
        for(int i=0 ; i<skin_mask.size();i++){
            rectangle(displayROI,skin_mask[i],Scalar(0,0,0),-1);
        }
        /***********************************************/
        string log_msg = "cellphone_detection::run() enter4(obj.size():";
        log_msg+= to_string(obj.size()); log_msg+=")";
        myLog->printLog(logAddMem(log_msg));
        /***************** 更新 *******************************/

        //=============== 單執行緒方法(Begin) ========================
//        skeleton *skeletonThread;
//        if(obj.size()>0){
//            skeletonThread = new skeleton(displayROI,obj[0],0,detect_phone_sensitivity);
//            skeletonThread->start(QThread::HighestPriority);
//            temp_Img.clear();
//            temp_ROI.clear();
//            while(1){
//                if(skeletonThread->isFinished()){
//                    temp_Img = skeletonThread->getImg();
//                    temp_ROI = skeletonThread->getRect();
//                    delete skeletonThread;
//                    skeletonThread = NULL;
//                    break;
//                }
//                this->msleep(10);
//            }
//            if(temp_Img.size()>0 && temp_ROI.size()>0){
//                for(int i=0;i<temp_Img.size();i++){
//                    vector<Rect> handpick;
//                    Mat hand_temp;
//                    cvtColor( temp_Img[i], hand_temp, CV_BGR2GRAY );
//                    equalizeHist( hand_temp,hand_temp );
//                    handdetection.detectMultiScale( hand_temp, handpick, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
//                    if(handpick.size()>0 && handpick.size()<2){
//                        for(int j=0;j<handpick.size();j++){
//                            CellPhone_thread->mutex_push(temp_Img[i](handpick[j]),temp_ROI[i]); // push 手機辨識thread
//                            /****************存圖片******************/
//                            imwrite(this->saveFilePath(counter),temp_Img[i](handpick[j]));
//                            /**************************************/
//                        }
//                    }
//                }
//            }
//        }
        //=============== 單執行緒方法(End) ========================

        //=============== 多執行緒方法(Begin) ========================
        skeleton *skeletonThread[100]={NULL};
        for(int i=0;i<(int)obj.size();i++){
            cout<< skeletonThread[i]->currentThread() <<" skeletonThread start"<<endl;
            skeletonThread[i] = new skeleton(displayROI,obj[i],i,detect_phone_sensitivity);
            skeletonThread[i]->start(QThread::HighestPriority);
        }
        for(int i =0 ;i<(int)obj.size();i++){
            temp_Img.clear();
            temp_ROI.clear();
            while(1){
                if(skeletonThread[i]->isFinished()){
                    temp_Img = skeletonThread[i]->getImg();
                    temp_ROI = skeletonThread[i]->getRect();
                    delete skeletonThread[i];
                    skeletonThread[i] = NULL;
                    break;
                }
                this->msleep(10);
            }
            if(temp_Img.size()>0 && temp_ROI.size()>0){
                for(int i=0;i<temp_Img.size();i++){
                    vector<Rect> handpick;
                    Mat hand_temp;
                    cvtColor( temp_Img[i], hand_temp, CV_BGR2GRAY );
                    equalizeHist( hand_temp,hand_temp );
                    handdetection.detectMultiScale( hand_temp, handpick, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
                    if(handpick.size()>0 && handpick.size()<2){
                        for(int j=0;j<handpick.size();j++){
                            Rect temp_rect = handpick[j];
                            temp_rect.x = temp_rect.x-15;
                            temp_rect.y = temp_rect.y-15;
                            temp_rect.width = temp_rect.width+30;
                            temp_rect.height = temp_rect.height+30;
                            temp_rect = rectSize(temp_Img[i],temp_rect);
                            CellPhone_thread->mutex_push(temp_Img[i](temp_rect),temp_ROI[i]); // push 手機辨識thread
                            /****************存圖片******************/
                            imwrite(this->saveFilePath(counter),temp_Img[i](temp_rect));
                            /**************************************/
                        }
                    }
                }
            }
        }
        //================ 多執行緒方法(End) ========================

         myLog->printLog(logAddMem("cellphone_detection::run() enter5"));
        display::drawRectsDifColor(displayROI,obj);//顯示人物藍框
        /*******************顯示紅框************************************
        Rect ROI;
        if(CellPhone_thread->GetRoi(ROI)){
            CellPhone_thread->result = false;
            for(int i=0;i<(int)obj.size();i++){
                if(ROI.x > obj[i].x && ROI.x + ROI.width < obj[i].x +obj[i].width
                        && ROI.y > obj[i].y && ROI.y + ROI.height <  obj[i].y +obj[i].height ){
                    ROI_timer=15;
                    Human_ROI = obj[i];
                }
            }
        }
        if(ROI_timer > 0 && CellPhone_thread->result_ROI == true ){
            double Min = 10000;
            int clost_obj=-1;
            for(int i=0;i<(int)obj.size();i++){
                if(sqrt(pow(obj[i].x-Human_ROI.x,2)+pow(obj[i].y-Human_ROI.y,2)) < Min ){
                    Min = sqrt(pow(obj[i].x-Human_ROI.x,2)+pow(obj[i].y-Human_ROI.y,2));
                    clost_obj = i;
                }
            }
            if(clost_obj>=0){
                Human_ROI = obj[clost_obj];
                rectangle(displayROI,Human_ROI,CV_RGB(255,0,0),5);
            }
            if(ROI_timer == 1 ){
                CellPhone_thread->Set_result_ROI(false);
            }
        }
        ROI_timer--;
        /**********************************************************/
        imshow("catch",displayROI);
        waitKey(1);
        counter++;
        if(counter==numeric_limits<int>::max() - 500)
            counter = 0;
    }//end of while

    //qDebug() << CellPhone_thread->currentThread() <<" Stop the cellphone_detection thread 1";
    CellPhone_thread->stop();
//    CellPhone_thread->wait();
//    while(1){
//        if(CellPhone_thread->isFinished()){
//            delete CellPhone_thread;
//            CellPhone_thread = NULL;
//            cout<< CellPhone_thread->currentThread() <<" CellPhone_thread end"<<endl;
//            break;
//        }
//        msleep(10);
//    }
    //qDebug() << CellPhone_thread->currentThread() << " Stop the cellphone_detection thread 2";
    destroyWindow("catch");
    myLog->printLog(logAddMem("cellphone_detection::run() exit2(End)"));
//    destroyWindow("BG SUB");
//    destroyWindow("SkinMask");
}

string cellphone_detection::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}

string cellphone_detection::saveFilePath(const int count)
{
    struct tm newtime;
    __time64_t long_time;
    errno_t err;
    // Get time as 64-bit integer.
    _time64( &long_time );
    // Convert to local time.
    err = _localtime64_s( &newtime, &long_time );
    if (err)
    {
        printf("Invalid argument to _localtime64_s.");
        exit(1);
    }
    string filePath="cell_phone/";
    filePath += to_string(newtime.tm_year + 1900);
    filePath += "-"; filePath += to_string(newtime.tm_mon + 1);
    filePath += "-"; filePath += to_string(newtime.tm_mday);
    filePath += " "; filePath +=to_string(newtime.tm_hour);
    filePath += "-"; filePath +=to_string(newtime.tm_min);
    filePath += "-"; filePath +=to_string(newtime.tm_sec);
    filePath += "_"; filePath +=to_string(count); filePath +=".jpg";
    return filePath;
}

