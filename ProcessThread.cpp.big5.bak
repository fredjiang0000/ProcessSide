#include "ProcessThread.h"
#include "ImgProcess.h"
#include <QtCore>
#include <QtDebug>
#include <ctime>  //for time log
#include <limits> //for counter if(max)
#include <sstream> //for try and catch
#include <string>
#include <fstream> //for log file
#include "time.h"//estimate the function exe time
#include "header.h"
#include "warningmessage.h"

bool warningmessage::Result = false;
//Constructor建構子
//目的：從Buffer中Pop Frame出來,對影像做處理後,傳送到顯示端
//參數：1.主程式視窗 2.Frame Buffer 3.監視器解析度寬
//      4.監視器解析度長 5.監視器FPS 6.監視器地點
//      7.顯示端IP 8.顯示端Port
ProcessThread::ProcessThread(MainWindow *mainwindow,ConcurrentQueue<Mat>* queue,
                             int frame_width, int frame_height, int FPS,char* cam_location,
                             char* server_ip,const int server_port):QThread()
{
    myLog = new outputLog("ProcessThread.log");
    myLog->printLog(logAddMem("ProcessThread::ProcessThread() enter"));
    //**************** 開啟Winsocket-DLL ************************
    int iResult;
    WSAData wsaData;
    WORD DLLVSERION;
    DLLVSERION = MAKEWORD(2,2);//Winsocket-DLL 版本

    //用 WSAStartup 開始 Winsocket-DLL
    iResult = WSAStartup(DLLVSERION, &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        cout << "WSAStartup failed with error: %d\n" << iResult << endl;
        this->quit();
    }
    //***********************************************************
    //*********** 初始化參數 ***************
    this->thread_main_wd = mainwindow;
    this->frame_queue = queue;
    this->cam_location = cam_location;
    this->frameWidth = frame_width;
    this->frameHeight = frame_height;
    this->Cap_FPS = FPS;
    this->server_ip = server_ip;
    this->server_port = server_port;
    this->log_filename = "";
    this->roiAlert = false;
    this->phoneAlert = false;
    this->roiTiming = false;
    this->phoneTiming = false;
    this->doStop = false;
    this->pop_counter = 0;
    this->empty_counter = 0;
    this->push_counter = 0;
    this->Event_Detect_Code = 0;
    this->detectObjectInOutROI = NULL;
    /* Camshift parameters */
    backprojMode = false;
    hsize = 256;
    objCount = 0;
    breakCounter = 0;
    lastMovingNum = 0;
    breakCheck = 0;
    textFontScale = 2;
    textThickness = 2;
    /* Update Processing FPS */
    proc_sampleNumber = 0;
    proc_fpsSum = 0;
    proc_averageFPS = 0;
    proc_fps.clear();
    //初始化壓縮參數
    params.push_back(CV_IMWRITE_JPEG_QUALITY);
    params.push_back(75);   // that's percent, so 100 == no compression, 1 == full

    //update the mainwindow's Log
    QObject::connect(this,SIGNAL(PrintMyLog(QString,const int)),
                     thread_main_wd,SLOT(UpdateMyLog(QString,const int)));
    //delete the thread when quit
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    //    //偵測手機行為
    phone_thread = NULL;
    //    namedWindow("fore_ground",CV_WINDOW_NORMAL);
    myLog->printLog(logAddMem("ProcessThread::ProcessThread() exit"));
}
//Destructor 解構子
ProcessThread::~ProcessThread()
{
//    Output_Log("Destructure Processing Thread!");
    myLog->printLog(logAddMem("ProcessThread::~ProcessThread()"));
    delete this->myLog;
    myLog = NULL;
}

//==============確認監視器IP是否活著=========================
bool ProcessThread::isIPAddressAlive(const QString ping_ip, const int port)
{
    myLog->printLog(logAddMem("ProcessThread::isIPAddressAlive() enter"));
    // this is not blocking call
    QSocket.connectToHost(ping_ip,port);
    // we need to wait...
    if(!QSocket.waitForConnected(5000))
    {
        qDebug() << "Error: " << QSocket.errorString();
        myLog->printLog(logAddMem("ProcessThread::isIPAddressAlive() exit1(IP is dead)"));
        return false;
    }
    else
    {
        myLog->printLog(logAddMem("ProcessThread::isIPAddressAlive() exit2(IP is alive)"));
        return true;
    }
}
//=============== 初始化有關於影像資訊的參數 ====================
void ProcessThread::initialization(VideoCapture& cap)
{
    myLog->printLog(logAddMem("ProcessThread::initialization() enter"));
    //若Width超過800 , 則固定傳送至顯示端的影像解析度為 800 x 600
    fixed_resolution = false;
    if(frameWidth > 800)
    {
        width_ratio = (double)frameWidth / 800;
        height_ratio = (double)frameHeight / 600;
        fixed_resolution = true;
    }
    string msg = "監視器原始解析度:";
    msg+=to_string(frameWidth); msg+="X"; msg+=to_string(frameHeight);
    msg+=" FPS:"; msg+=to_string(Cap_FPS);
//    Output_Log(msg);
    Log_Msg = QString::fromStdString(msg);
    emit PrintMyLog(Log_Msg,1);
    //ROI Set(用途：偵測指定區域物體移動的背景更新和剪出前景)
    ROIs.clear();
    Rect temp(0,0,frameWidth,frameHeight);
    ROIs.push_back(temp);
    /* 初始化設定偵測區域 */
    //設定指定區域偵測物體(比較前景和背景差異的Rect)
    ParkingLot_ROI.clear();
    //設定指定區域偵測物體(使用者畫的座標)
    ParkingLot_Display.clear();
    /* Initialize background */
    BackgroundInitialize(cap, frame_bg, frame_ref, frame_last, ROIs);
    myLog->printLog(logAddMem("ProcessThread::initialization() exit"));
}
//=================連線至顯示端===========================
bool ProcessThread::Connect_to_Server(char* server_ip, int server_port)
{
    myLog->printLog(logAddMem("ProcessThread::Connect_to_Server() enter"));
    //========清空傳給顯示端的Log,為了避免一直重連產生過多的訊息========
    if(Log_for_display.size() > 15)
        Log_for_display.clear();

    string log_msg = "連線至顯示端中...";
//    Output_Log(log_msg);
    Log_for_display.push_back(log_msg);
    //Initialize sockets
    if ((clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        log_msg = "開啟Socket失敗! "; log_msg += WSAGetLastError();
        Log_for_display.push_back(log_msg);
        cout << "Fail to open the Socket " << WSAGetLastError() << endl;
//        Output_Log(log_msg);
        Log_Msg = "開啟Socket失敗! "; Log_Msg += WSAGetLastError();
        emit PrintMyLog(Log_Msg,2);
        printf("%s %s Fail to open the Socket error:%ld.\n",get_current_time().c_str(),this->cam_location,WSAGetLastError());
        myLog->printLog(logAddMem("ProcessThread::Connect_to_Server() exit1(Open Socket Error)"));
        return false;
    }
    //Set any options
    BOOL option =TRUE;
    if( (setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&option,sizeof(BOOL)) == -1 )||
            (setsockopt(clientSock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&option,sizeof(BOOL))  == -1 ) ){
        printf("%s %s Error setting options:%ld.\n",get_current_time().c_str(),this->cam_location,WSAGetLastError());
        WSACleanup();
        myLog->printLog(logAddMem("ProcessThread::Connect_to_Server() exit2(Setting Option Error)"));
        return false;
    }
    //================= Connect to the server ============================
    ZeroMemory( &serverAddr, sizeof(serverAddr) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);
    serverAddr.sin_addr.s_addr = inet_addr(server_ip);

    serverAddrLen = sizeof(serverAddr);
    if (::connect(clientSock, (SOCKADDR*)&serverAddr, serverAddrLen) < 0) {
        log_msg = "連線至顯示端失敗!"; log_msg += WSAGetLastError();
        Log_for_display.push_back(log_msg);
//        Output_Log(log_msg);
        Log_Msg = "連線至顯示端失敗!"; Log_Msg += WSAGetLastError();
        emit PrintMyLog(Log_Msg,2);
        printf("%s %s Fail to Connect to DisplaySide! error:%ld.\n",get_current_time().c_str(),this->cam_location,WSAGetLastError());
        myLog->printLog(logAddMem("ProcessThread::Connect_to_Server() exit3(Connect Error)"));
        return false;
    }
    else//成功連線至顯示端
    {
//        Output_Log("成功連線至顯示端");
        Log_Msg = "成功連線至顯示端";
        emit PrintMyLog(Log_Msg,0);
        Log_for_display.push_back("成功連線至顯示端");
        myLog->printLog(logAddMem("ProcessThread::Connect_to_Server() exit4(Success)"));
        return true;
    }
}
//停止Push並一直嘗試重新連線至顯示端直到成功連上
void ProcessThread::reConnectToDisplay(char *server_ip, int server_port)
{
    string log_msg="";
    myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() enter1"));
    //一直嘗試重新連線至顯示端直到成功連上
    while(1)
    {
        ////////////////////////////////
        // Stop thread if doStop=TRUE //
        ////////////////////////////////
        doStopMutex.lock();
        if(doStop)
        {
            doStopMutex.unlock();
            myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() enter2(doStop)"));
            break;
        }
        doStopMutex.unlock();

        this->clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        ZeroMemory( &this->serverAddr, sizeof(this->serverAddr) );
        this->serverAddr.sin_family = AF_INET;
        this->serverAddr.sin_port = htons(server_port);
        this->serverAddr.sin_addr.s_addr = inet_addr(server_ip);
        this->serverAddrLen = sizeof(this->serverAddr);
        int iResult = ::connect(this->clientSock, (SOCKADDR*)&this->serverAddr, this->serverAddrLen);

        if ( iResult == SOCKET_ERROR)
        {
            int iError = WSAGetLastError();
            if(iError  == WSAEWOULDBLOCK){
                myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() enter3)"));
                fd_set Write, Err;
                TIMEVAL Timeout;
                int TimeoutSec = 10; // timeout after 10 seconds
                FD_ZERO(&Write);
                FD_ZERO(&Err);
                FD_SET(clientSock, &Write);
                FD_SET(clientSock, &Err);
                Timeout.tv_sec = TimeoutSec;
                Timeout.tv_usec = 0;
                iResult = select(0,//ignored
                                 NULL,      //read
                                 &Write,    //Write Check
                                 &Err,      //Error Check
                                 &Timeout);
                if(iResult == 0)
                {
                    myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() enter4(Timeout)"));
                    this->Log_Msg = "重新連線至顯示端失敗!(Timeout)";
                    emit PrintMyLog(this->Log_Msg,2);
                }
                else
                {
                    if(FD_ISSET(clientSock, &Write))
                    {
                        myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() enter5(Succeed)"));
                        this->Log_Msg = "成功重新連線至顯示端(Select succeed)";
                        emit PrintMyLog(this->Log_Msg,0);
                        break;
                    }
                    if(FD_ISSET(clientSock, &Err))
                    {
                        myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() enter6(Select Error)"));
                        this->Log_Msg = "重新連線至顯示端失敗!(Select error)";
                        emit PrintMyLog(this->Log_Msg,2);
                    }
                }
            }
            else{
                log_msg= "ProcessThread::reConnectToDisplay() enter7(Failed)(Error:";
                log_msg+= to_string(WSAGetLastError()); log_msg+=")";
                myLog->printLog(logAddMem(log_msg));
                this->Log_Msg = "重新連線至顯示端失敗! Error:"; this->Log_Msg += WSAGetLastError();
                emit PrintMyLog(this->Log_Msg,2);
            }
        }
        else{
            myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() enter8(Succeed)"));
            this->Log_Msg = "成功重新連線至顯示端";
            emit PrintMyLog(this->Log_Msg,0);
            break;
        }
    }
    myLog->printLog(logAddMem("ProcessThread::reConnectToDisplay() exit"));
}
//============FPS的更新==========================
void ProcessThread::proc_updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed>0)
    {
        proc_fps.enqueue((int)1000/timeElapsed);
        // Increment sample number
        proc_sampleNumber++;
    }
    // Maximum size of queue is DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH
    if(proc_fps.size()>CAPTURE_FPS_STAT_QUEUE_LENGTH)
        proc_fps.dequeue();
    // Update FPS value every DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH samples
    if((proc_fps.size()==CAPTURE_FPS_STAT_QUEUE_LENGTH)&&(proc_sampleNumber==CAPTURE_FPS_STAT_QUEUE_LENGTH))
    {
        // Empty queue and store sum
        while(!proc_fps.empty())
            proc_fpsSum+=proc_fps.dequeue();
        // Calculate average FPS
        proc_averageFPS=proc_fpsSum/CAPTURE_FPS_STAT_QUEUE_LENGTH;
        // Reset sum
        proc_fpsSum=0;
        // Reset sample number
        proc_sampleNumber=0;
    }
}

//*********影像處理功能─設定特定區域物體偵測**************
void ProcessThread::ROI_Detect_Objects(const Mat& frame)
{
    myLog->printLog(logAddMem("ProcessThread::ROI_Detect_Objects() enter"));
    ParkingLot_ROI_Not_Update.clear();//清空不更新的區域
    Mat fore_ground = Get_ForeGround(frame);//取得前景

    //=========== 作形態學處理,膨脹收縮 ===========
    //            Mat element = getStructuringElement( 2, Size( MORPH_SIZE,MORPH_SIZE ), Point( 2,2 ) );
    //            morphologyEx(frame_median, frame_morph, MORPH_CLOSE, element , Point( -1,-1 ) , MORPH_ITERATION);
    //            imshow("frame_motion",frame_motion);
    //            imshow("Back_ground",frame_bg);
    //            imshow("frame_median",frame_median);
    //            imshow("frame_morph",frame_morph);

    //    imshow("fore_ground",fore_ground);
    //    waitKey(1);
    this->roiAlert = false;
    //比較指定區域的前景和背景(灰階比較)
    for(int n=0; n<ParkingLot_ROI.size(); n++)
    {
        int diffPixels = 0;//前景和背景差異Pixels數目參數初始歸零
        //將指定區域的前景部分擷取出來
        Mat diff_ROI(frame(ParkingLot_ROI[n]).size(),CV_8UC1,Scalar(0));
        fore_ground(ParkingLot_ROI[n]).copyTo(diff_ROI);

        //計算指定區域前景的部分,有多少pixel和背景不同
        for(int i = 0; i < diff_ROI.rows; i++)
        {
            const unsigned char* row  = diff_ROI.ptr<unsigned char>(i);
            for(int j = 0; j < diff_ROI.cols; j++)
            {
                if(row[j]==255)
                    diffPixels++;
            }
        }
        detectObjectInOutROI[n].setDiffPixels(diffPixels);//設定差異門檻值
        cout  << "ROI("<< n << ") Width:" << ParkingLot_ROI[n].width << " Height:" << ParkingLot_ROI[n].height
              <<" DiffThreshold:" << detectObjectInOutROI[n].getDiffThreshold() << " DiffPixels:" << detectObjectInOutROI[n].getDiffPixels() << endl;

        //若前景 和 背景 pixel的差異度超過設定的門檻
        if (detectObjectInOutROI[n].getDiffPixels()  >= detectObjectInOutROI[n].getDiffThreshold()){
            cout << "Detect ROI(" << n << ") is occupied!" << endl;
            detectObjectInOutROI[n].setAlert(true);
            recordObjectInOut(n);
            this->roiAlert = true;
        }
        else//若前景和背景差異程度小於門檻值
        {
            detectObjectInOutROI[n].setAlert(false);
            recordObjectDisappear(n);
        }
    }//[End]比較指定區域的前景和背景(灰階比較)
    myLog->printLog(logAddMem("ProcessThread::ROI_Detect_Objects() exit"));
}

bool ProcessThread::is_ASE_Car(Mat input_frame)
{
    IplImage img = IplImage(input_frame);//轉Mat to IplImage(!共用記憶體位址)
    TruckRecognize truck(&img);
    if(truck.isCompanyCar())//判斷是公司車
        return true;
    else//判斷為非公司車
        return false;
}
//取得前景(灰階)
Mat ProcessThread::Get_ForeGround(const Mat &src_frame)
{
    myLog->printLog(logAddMem("ProcessThread::Get_ForeGround() enter"));
    frame_gray = Grayscale(src_frame);//轉灰階
    frame_gray.copyTo(frame_motion);//Copy
    MotionMask(frame_motion,frame_bg,ROIs);//取得前景
    medianBlur(frame_motion, frame_median, 5);//過濾雜訊
    myLog->printLog(logAddMem("ProcessThread::Get_ForeGround() exit"));
    return frame_median;
}
//只更新指定區域以外的背景
void ProcessThread::Update_BG_Not_ROI()
{
    myLog->printLog(logAddMem("ProcessThread::Update_BG_Not_ROI() enter"));

    cout  << "ParkingLot_ROI_Not_Update.size()" <<  ParkingLot_ROI_Not_Update.size() << endl;
    Mat temp[ParkingLot_ROI_Not_Update.size()];
    for(int n=0; n<ParkingLot_ROI_Not_Update.size(); n++)
    {
        frame_bg(ParkingLot_ROI_Not_Update[n]).copyTo(temp[n]);
    }
    BackgroundUpdate(&frame_bg,&frame_ref,frame_gray,ROIs);
    for(int n=0; n<ParkingLot_ROI_Not_Update.size(); n++)
    {
        temp[n].copyTo(frame_bg(ParkingLot_ROI_Not_Update[n]));
    }
    myLog->printLog(logAddMem("ProcessThread::Update_BG_Not_ROI() exit"));
}
//有物體進出指定區域,記錄Log事件,標記已記錄過警報
void ProcessThread::recordObjectInOut(const int index)
{
    myLog->printLog(logAddMem("ProcessThread::recordObjectInOut() enter"));
    string log_msg;
    if(!detectObjectInOutROI[index].getLogAlertRecord()){//沒記錄過才會記錄
        cout << "[警報] 有物體進出指定區域" << endl;
        log_msg = "[警報] 有物體進出指定區域";
        log_msg+= to_string(index+1);
//        Output_Log(log_msg);
        Log_for_display.push_back(log_msg);
        emit PrintMyLog(QString("[警報] 有物體進出指定區域%1").arg(QString::number(index+1)),2);
        detectObjectInOutROI[index].setLogAlertRecord(true);//已記錄警報
    }
    myLog->printLog(logAddMem("ProcessThread::recordObjectInOut() exit"));
}
//物體進出指定區域過了一段時間,印出Log訊息,將警報記錄關閉
void ProcessThread::recordObjectDisappear(const int index)
{
    myLog->printLog(logAddMem("ProcessThread::recordObjectDisappear() enter"));
    string log_msg;
    if(detectObjectInOutROI[index].getLogAlertRecord()){
        cout << "[一般] 有物體經過或進出指定區域%1過了一段時間" << endl;
        log_msg = "[一般] 有物體經過或進出指定區域%1過了一段時間";
        log_msg+= to_string(index+1);
//        Output_Log(log_msg);
        Log_for_display.push_back(log_msg);
        emit PrintMyLog(QString("[一般] 有物體經過或進出指定區域%1過了一段時間").arg(QString::number(index+1)),1);
        detectObjectInOutROI[index].setLogAlertRecord(false);
    }
    myLog->printLog(logAddMem("ProcessThread::recordObjectDisappear() exit"));
}


//畫出每個超過threshold的指定區域
void ProcessThread::Draw_ROI_Detect(Mat& frame)
{
    myLog->printLog(logAddMem("ProcessThread::Draw_ROI_Detect() enter"));
    for(int n=0; n<ParkingLot_ROI.size(); n++)
    {
        if (detectObjectInOutROI[n].getDiffPixels()  > detectObjectInOutROI[n].getDiffThreshold())
        {
            for (int i = n*4; i < n*4 + 4; i++) {
                if(i%4==3)//第四點連到第一點
                {
                    line(frame,ParkingLot_Display[i],ParkingLot_Display[i-3],Scalar(0,0,255),5);
                    continue;
                }
                line(frame,ParkingLot_Display[i],ParkingLot_Display[i+1],Scalar(0,0,255),5);
            }
        }
    }
    myLog->printLog(logAddMem("ProcessThread::Draw_ROI_Detect() exit"));
}

//根據事件發送Email
//參數： 1.要副檔的Frame 2.寄信的時間間隔
void ProcessThread::doSendEmailByEvent(const Mat sendFrame, const int timeInterval)
{
    myLog->printLog(logAddMem("ProcessThread::doSendEmailByEvent() enter"));
    //===== 偵測指定區域物體事件 觸發警報=====
    if(Event_Detect_Code % 10 >= 1 && roiAlert)
    {
        //儲存整張圖片
//        imwrite(this->saveFilePath("cell_phone/entire/designated area/",this->pop_counter),sendFrame);
        //尚未開始計時 => 開始計時並寄一封信
        if(!this->roiTiming){
            this->roiAlertBegin = clock();//開始計時
            this->roiTiming = true;//紀錄已經開始計時
            smtpSendEmail(sendFrame,"ROI",get_current_time());//Send SMTP Email
        }
        else{//繼續計時
            this->roiAlertEnd = clock();
            double elapsed_secs = double(roiAlertEnd - roiAlertBegin) / CLOCKS_PER_SEC;
            qDebug() << "ROI elapsed_secs:" << elapsed_secs;
            if(elapsed_secs > timeInterval){
                this->roiTiming = false;//計時完畢,可以重新計時
            }
        }
    }
    //===== 偵測使用手機事件 觸發警報 =====
    if(Event_Detect_Code % 100 >= 10 && phoneAlert)
    {
        //儲存整張圖片
//        imwrite(this->saveFilePath("cell_phone/entire/using phone/",this->pop_counter),sendFrame);
        cout << "doSendEmailByEvent phoneAlert Trigger" << endl;
        //尚未開始計時 => 開始計時並寄一封信
        if(!this->phoneTiming){
            cout << "doSendEmailByEvent phoneAlert first Trigger" << endl;
            this->phoneAlertBegin = clock();//開始計時
            this->phoneTiming = true;//紀錄已經開始計時
            smtpSendEmail(sendFrame,"UsePhone",get_current_time());//Send SMTP Email
        }
        else{//繼續計時
            this->phoneAlertEnd = clock();
            double elapsed_secs = double(phoneAlertEnd - phoneAlertBegin) / CLOCKS_PER_SEC;
            cout << "doSendEmailByEvent Phone elapsed_secs:" << elapsed_secs << endl;
            if(elapsed_secs > timeInterval){
                this->phoneTiming = false;//計時完畢,可以重新計時
            }
        }
    }
    myLog->printLog(logAddMem("ProcessThread::doSendEmailByEvent() exit"));
}

//傳送Email給設定(附檔：當下的frame)
//參數： 1.當下的Frame 2.偵測事件類型(ex:指定區域、使用手機...) 3.發生時間
ProcessThread::smtpSendEmail(const Mat cur_frame,const string event_type, const string occurTime)
{

    myLog->printLog(logAddMem("ProcessThread::smtpSendEmail() enter"));
    qDebug() << "===== smtpSendEmail =====";
    QThread *thread = new QThread();
    int port = this->smtpPort;//目前沒用到(寫死Port = 25)
    Smtp *smtp = new Smtp(this->smtpSender,this->smtpPassword,this->smtpServer,
                          this->smtpReceiver,cur_frame,event_type,occurTime);

    smtp->moveToThread(thread);
    connect(thread, SIGNAL(started()), smtp, SLOT(doSendMail()));
    connect(smtp, SIGNAL(workFinished()), thread, SLOT(quit()));
    //automatically delete thread and task object when work is done:
    connect(thread, SIGNAL(finished()), smtp, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
    myLog->printLog(logAddMem("ProcessThread::smtpSendEmail() exit"));
}

string ProcessThread::saveFilePath(const string DirPath,const int count)
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
    string filePath=DirPath;
    filePath += to_string(newtime.tm_year + 1900);
    filePath += "-"; filePath += to_string(newtime.tm_mon + 1);
    filePath += "-"; filePath += to_string(newtime.tm_mday);
    filePath += " "; filePath +=to_string(newtime.tm_hour);
    filePath += "-"; filePath +=to_string(newtime.tm_min);
    filePath += "-"; filePath +=to_string(newtime.tm_sec);
    filePath += "_"; filePath +=to_string(count); filePath +=".jpg";
    return filePath;
}

//*********影像處理功能─追蹤軌跡且記錄發生的事件**************
//Author:Szu-Cheng Shen
//Date:2015/9/11
//Functions:
//     1. moving object detection
//     2. object tracking
//     3. event recording
//     4. special event detection
void ProcessThread::Motion_Detect()
{
    //每[NUM_FRAME_PROCESS]張Frame處理一次
    if (pop_counter % NUM_FRAME_PROCESS == 0)
    {
        trackBox2_vector.clear();//清空要畫在畫面上的Rect
        /* 刪除消失過久之追蹤物件、將物件記錄為事件、判斷未下車的特殊事件 */
        for (int k=0; k < oldObjects.size(); ++k) {
            if (pop_counter - oldObjects[k].trajectoryTime.back() > DISSAPEAR_TIME) {
                int eventType = RecordObject(oldObjects[k], recordName, exitHuman, exitVehicle);
                if (eventType == 1) {  //停車事件，來判斷是否停好車後沒人下車 以及 是否停到限制區域

                    int flagGetOff = 0;         //有無下車FLAG 0=無 1=有
                    int tSize = oldObjects[k].trajectoryTime.size();
                    float avgHeight = 1.0*oldObjects[k].sumHeight / tSize;
                    float avgWidth = 1.0*oldObjects[k].sumWidth / tSize;
                    int tempCarX =oldObjects[k].trajectoryPoint.back().x - avgWidth*0.5;
                    int tempCarY =oldObjects[k].trajectoryPoint.back().y - avgHeight*0.5;
                    Rect carPosition(tempCarX,tempCarY,avgWidth,avgHeight);
                    cars.push_back(carPosition);

                    for (int l=k; l< oldObjects.size(); ++l) {
                        if (checkEvent(oldObjects[k])) continue;        //非事件直接略過
                        if (!checkHuman(oldObjects[k]))  continue;      //非人直接略過
                        int tempTimes = 24;
                        if (oldObjects[l].trajectoryTime.size() < tempTimes) tempTimes = oldObjects[l].trajectoryTime.size();
                        for (int m=0; m<tempTimes; ++m) {
                            if (IntersectionCheck(oldObjects[l].trajectoryRect[m],carPosition)) { //物件出現的前幾秒有與車子最後位置相交
                                flagGetOff = 1;
                                break;
                            }
                        }
                        if (flagGetOff == 1) break;
                    }

                    for (int l=0; l< objects.size(); ++l) {
                        if (flagGetOff == 1) break;
                        if (checkEvent(objects[l])) continue;        //非事件直接略過
                        if (!checkHuman(objects[l]))  continue;      //非人直接略過
                        int tempTimes = 24;
                        if (objects[l].trajectoryTime.size() < tempTimes) tempTimes = objects[l].trajectoryTime.size();
                        for (int m=0; m<tempTimes; ++m) {
                            if (IntersectionCheck(objects[l].trajectoryRect[m],carPosition)) { //物件出現的前幾秒有與車子最後位置相交
                                flagGetOff = 1;
                                break;
                            }
                        }
                    }

                    if (flagGetOff == 0) {
                        carsNotGetOff.push_back(carPosition);
                    }

                }
                if (eventType == 3) {   //車子開走，把該區域車子標記取消
                    for (int l=0; l<cars.size(); l++) {
                        if (pointRectDistance(oldObjects[k].trajectoryPoint.back(), cars[l]) == 0 ) { //
                            cars.erase(cars.begin()+l);
                            --l;
                        }
                    }
                }
                oldObjects.erase(oldObjects.begin()+k);
                k -= 1;
                continue;
            }
        }
        /* 刪除消失過久之追蹤物件、將物件記錄為事件、判斷未下車的特殊事件 (end)*/

        /* moving object detection */
        frame_gray = Grayscale(frame);
        BackgroundUpdate(&frame_bg,&frame_ref,frame_gray,ROIs);
        frame_gray.copyTo(frame_motion);
        MotionMask(frame_motion,frame_bg,ROIs);
        medianBlur(frame_motion, frame_median, 5);
        Mat element = getStructuringElement( 2, Size( MORPH_SIZE,MORPH_SIZE ), Point( 2,2 ) );
        morphologyEx(frame_median, frame_morph, MORPH_CLOSE, element , Point( -1,-1 ) , MORPH_ITERATION);

        cv::Mat output = cv::Mat::zeros(frame.size(), CV_8UC3);
        cv::Mat tempmask = cv::Mat::zeros(frame.size(), CV_8UC1);
        std::vector < std::vector<cv::Point2i > > blobs;
        FindBlobs(frame_morph, blobs);
        // 把blobs畫上各自不同的顏色 (for 顯示)
        for(size_t i=0; i < blobs.size(); i++) {
            unsigned char r = 255 * (rand()/(1.0 + RAND_MAX));
            unsigned char g = 255 * (rand()/(1.0 + RAND_MAX));
            unsigned char b = 255 * (rand()/(1.0 + RAND_MAX));
            for(size_t j=0; j < blobs[i].size(); j++) {
                int x = blobs[i][j].x;
                int y = blobs[i][j].y;

                tempmask.at<uchar> (y,x) = 255;
                output.at<cv::Vec3b>(y,x)[0] = b;
                output.at<cv::Vec3b>(y,x)[1] = g;
                output.at<cv::Vec3b>(y,x)[2] = r;
            }
        }
        /* moving object detection (end)*/

        /* 將frame從RGB色彩空間轉成HSV */
        frame.copyTo(image);
        cvtColor(image, hsv, CV_BGR2HSV);
        inRange(hsv, Scalar(0, SMIN, MIN(VMIN,VMAX)), Scalar(180, 256, MAX(VMIN, VMAX)), mask); // h = 0~180, s = smin~256, v = vmin~vmax
        mask = mask & tempmask; //只取有moving blob的點

        if ( ((int)blobs.size()-lastMovingNum) > BREAK_THRESHOLD) { //這個frame突然多出數個物件，判斷為一物件破碎，待物件數變少或到達一定次數才加入
            ++breakCounter;
            breakCheck = 1;
            if (breakCounter > BREAK_FRAMES_LIMIT) {
                breakCounter = 0;
                breakCheck = 0;
                lastMovingNum = blobs.size();
            }
            //cout << "* HOLD: " << breakCounter << endl;
        }
        else {
            lastMovingNum = blobs.size();
            breakCheck = 0;
        }

        /* 檢查每一個抓出的blob是否為新物件，是的話就加入追蹤，否的話則紀錄重疊到的物件 */
        int rectblobs[blobs.size()][4];
        for(size_t i=0; i < blobs.size(); i++) {
            tx_min = tx_max = blobs[i][0].x;
            ty_min = ty_max = blobs[i][0].y;

            for(size_t j=1; j < blobs[i].size(); j++) {         //把每一個object用rect框起來
                if (blobs[i][j].x < tx_min) tx_min = blobs[i][j].x;
                if (blobs[i][j].y < ty_min) ty_min = blobs[i][j].y;
                if (blobs[i][j].x > tx_max) tx_max = blobs[i][j].x;
                if (blobs[i][j].y > ty_max) ty_max = blobs[i][j].y;
            }
            rectblobs[i][0] = tx_min;
            rectblobs[i][1] = ty_min;
            rectblobs[i][2] = tx_max;
            rectblobs[i][3] = ty_max;
            Rect ithRect(tx_min,ty_min,tx_max-tx_min+1,ty_max-ty_min+1);

            int check = 0;
            vector <int> overLapObjects;
            for (int k = 0; k < objects.size(); ++k) {
                /*int x1 = objects[k].trackWindow.x;
                  int x2 = x1 + objects[k].trackWindow.width - 1;
                  int y1 = objects[k].trackWindow.y;
                  int y2 = y1 + objects[k].trackWindow.height - 1;*/
                if (IntersectionCheck(ithRect, objects[k].trackWindow) ) {//blob有和原本物件相交
                    //if ( (tx_min < x1 || tx_min < x2 ) && (tx_max > x1 || tx_max > x2) && (ty_min < y1 || ty_min < y2 ) && (ty_max > y1 || ty_max > y2)) {
                    check = 1;
                    objects[k].crossBlob.push_back(i);
                    overLapObjects.push_back(k);
                }
            }
            if (overLapObjects.size() > 1) { //一個以上物件重疊某blob，代表兩物件重疊，標記為重疊
                for (int k = 0; k < overLapObjects.size(); ++k) {
                    objects[ overLapObjects[k] ].overLap = 1;
                    cout << "@@overlap:" << overLapObjects[k] << endl;
                }

            }
            if (check == 0 && breakCheck == 0) //和目前追蹤中的物件都不一樣則加入新追蹤物件
            {
                trackingObject newObject;
                selection = Rect(tx_min,ty_min,tx_max-tx_min+1,ty_max-ty_min+1);

                calcWeightedHistogram(hsv, selection, newObject.hist, &tempmask);
                //normalize(newObject.hist, newObject.hist, 0, 255, CV_MINMAX);     //如果hsize不是255就需要normalize

                /* 檢查是否已有同個物件在追蹤 (被分割成多塊)，有的話就合併 */
                int checkSame = 0;
                Point newCenter = (selection.tl() + selection.br()) * 0.5;
                for (int k=0; k < objects.size(); ++k) {
                    Point kCenter = (objects[k].trackWindow.tl() + objects[k].trackWindow.br()) * 0.5;
                    if ( pow((kCenter.x-newCenter.x),2)+pow((kCenter.y-newCenter.y),2) < SAME_OBJECT_DISTANCE) {   //與原追蹤物件之中心距離小於根號SAME_OBJECT_DISTANCE
                        if (compareHist(objects[k].hist, newObject.hist, CV_COMP_CORREL) > SAME_HIST_THRESHOLD) {//檢查histgram相似度，相似度夠高判斷為同一個物件的多塊，合併histgram與搜索區域，剔除新物件
                            objects[k].hist += newObject.hist;
                            objects[k].trackWindow = objects[k].trackWindow | selection;
                            checkSame = 1;
                            break;
                        }
                    }
                }
                if (checkSame == 1) continue;
                /* 檢查是否已有同個物件在追蹤 (被分割成多塊)，有的話就合併 (fin)*/

                /* 檢查此物件是否和不久前消失之物件為同個，是的話就繼承 */
                int checkOld = 0;
                for (int k=0; k < oldObjects.size(); ++k) {
                    Point kCenter = oldObjects[k].trajectoryPoint.back();
                    int kAvgArea = oldObjects[k].sumArea/oldObjects[k].trajectoryTime.size();
                    if ( pow((kCenter.x-newCenter.x),2)+pow((kCenter.y-newCenter.y),2) < SAME_OBJECT_DISTANCE && abs(kAvgArea - selection.area()) < SAME_OBJECT_AREA_DIFFERENCE) {   //與消失物件之中心距離小於根號SAME_OBJECT_DISTANCE
                        if (compareHist(oldObjects[k].hist, newObject.hist, CV_COMP_CORREL) > SAME_HIST_THRESHOLD) {//檢查histgram相似度，相似度夠高判斷為方才消失之物件(遮擋又出現)，繼承其過去資料繼續追蹤
                            newObject.id = oldObjects[k].id;
                            newObject.trajectoryTime = oldObjects[k].trajectoryTime;
                            newObject.trajectoryPoint = oldObjects[k].trajectoryPoint;
                            newObject.trajectoryRect = oldObjects[k].trajectoryRect;
                            newObject.sumArea = oldObjects[k].sumArea;
                            newObject.sumHeight = oldObjects[k].sumHeight;
                            newObject.sumWidth = oldObjects[k].sumWidth;
                            newObject.sumBlobs = oldObjects[k].sumBlobs;
                            newObject.thief = oldObjects[k].thief;
                            oldObjects.erase(oldObjects.begin()+k);
                            checkOld = 1;
                            break;
                        }
                    }
                }
                /* 檢查此物件是否和不久前消失之物件為同個，是的話就繼承 (fin) */

                newObject.trackWindow = selection;

                /* 畫histogram的圖以便顯示的 (確認histogram用) */

                /*Marked out temporarily start ..............

                  newObject.histimg = Mat::zeros(200, 320, CV_8UC3);
                  newObject.histimg = Scalar::all(0);
                  int binW = newObject.histimg.cols / hsize;
                  Mat buf(1, hsize, CV_8UC3);
                  for( int i = 0; i < hsize; i++ )
                      buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                  cvtColor(buf, buf, CV_HSV2BGR);

                  for( int i = 0; i < hsize; i++ )
                  {
                      int val = saturate_cast<int>(newObject.hist.at<float>(i)*newObject.histimg.rows/255);
                      rectangle( newObject.histimg, Point(i*binW,newObject.histimg.rows),
                                 Point((i+1)*binW,newObject.histimg.rows - val),
                                 Scalar(buf.at<Vec3b>(i)), -1, 8 );
                  }

                  ............ Marked out temporarily end */

                if (checkOld == 0) {    //新物件
                    newObject.id = ++objCount;
                    newObject.sumArea = 0;
                    newObject.sumHeight = 0;
                    newObject.sumWidth = 0;
                    newObject.sumBlobs = 0;
                }
                newObject.overLap = 0;
                newObject.thief = 0;
                newObject.crossBlob.push_back(i);
                newObject.previousBlob = selection;
                objects.push_back(newObject);
                histgrams.push_back(newObject.hist);

                /** 判斷是否為原本未下車之人終於下車 **/

                Point newObjectCenter(selection.x+selection.width*0.5, selection.y+selection.height*0.5);
                for (int m=0;m<carsNotGetOff.size();++m) {
                    if (pointRectDistance (newObjectCenter, carsNotGetOff[m]) < THIEF_DISTANCE) {
                        carsNotGetOff.erase(carsNotGetOff.begin()+m);
                        m -= 1;
                    }
                }
            }
        }
        /* 檢查每一個抓出的blob是否為新物件，是的話就加入追蹤，否的話則紀錄重疊到的物件 (end)*/
        //
        /*** 各物件追蹤 ***/
        //

        int specialDriveAlive = 0;

        for (int i=0; i < objects.size(); ++i) {
            /* 沒有移動的物件符合所追蹤的結果，將此物件移到消失物件列並檢查是否為停在限制區域的車輛 */
            if (objects[i].crossBlob.size() == 0) {
                /**限制區域停放車輛檢查**/
                if (checkPosition(objects[i].trajectoryPoint.back(),specialAreas) ) {  //再限制區域內

                    if (checkHuman(objects[i]) == 0 ) {
                        specialAlert = 75;
                    }
                }
                /**限制區域停放車輛檢查end**/

                oldObjects.push_back(objects[i]);
                cout << "沒有移動的物件符合所追蹤的結果，將此物件移到消失物件列: [" << objects[i].id << "]" << endl;
                objects.erase(objects.begin()+i);
                i -= 1;

            }
            /* 沒有移動的物件符合所追蹤的結果，將此物件移到消失物件列並檢查是否為停在限制區域的車輛 (end) */

            /* 仍存活的物件，開始進行特殊事件判斷、追蹤以及畫出追蹤區域等功能 */
            else {

                /** 判斷繞行車子 **/
                int flagThief = checkThief(objects[i],cars);
                if (flagThief >= 0) {
                    objects[i].thief = 1;
                }
                /** 判斷繞行車子 **/
                /** 判斷特殊區域跑出 **/
                if (specialDriveAlert == 0 && objects[i].trajectoryPoint.size()>0) {
                    for (int j=0; j<specialAreas.size(); ++j) {
                        if ( pointRectDistance(objects[i].trajectoryPoint[0], specialAreas[j]) == 0) {   //起始點在特殊區域內
                            if (pointRectDistance(objects[i].trajectoryPoint.back(), specialAreas[j]) > 0) { //上一個畫面已經在外面了
                                specialDriveAlert = objects[i].id;
                                specialDriveAlive = 1;
                            }
                        }
                    }
                }
                else {
                    if (objects[i].id == specialDriveAlert)
                        specialDriveAlive = 1;
                }

                /* 先算出與此物件重疊之blob */    //rectblobs中會出現不存在的rect並且數值不合理，因此判斷不合理就不予採用

                float maxSim = 0;
                Rect movingBlob;
                while (objects[i].crossBlob.size() > 0) {
                    int j = objects[i].crossBlob.back();
                    if (rectblobs[j][0] < 0 || rectblobs[j][1] < 0 || rectblobs[j][2] < 0 || rectblobs[j][3] < 0 ||
                            rectblobs[j][0] >= frameWidth || rectblobs[j][2] >= frameWidth || rectblobs[j][1] >= frameHeight || rectblobs[j][3] >= frameHeight ||
                            rectblobs[j][0] > rectblobs[j][2] || rectblobs[j][1] > rectblobs[j][3]) {
                        continue;
                    }
                    Rect tempRect = Rect( rectblobs[j][0], rectblobs[j][1], rectblobs[j][2]-rectblobs[j][0]+1, rectblobs[j][3]-rectblobs[j][1]+1 );

                    /*[更新]:測試依照距離選取blob而非相似度 測試中 先替換掉下面三行 (更新:再替換回相似度) */
                    Mat tempHist;
                    calcWeightedHistogram(hsv, tempRect, tempHist, &tempmask);
                    //normalize(tempHist, tempHist, 0, 255, CV_MINMAX);     //如果hsize不是255就需要normalize
                    float tempSim = compareHist(objects[i].hist, tempHist, CV_COMP_CORREL);
                    if (tempSim > maxSim) {
                        maxSim = tempSim;
                        movingBlob = tempRect;
                    }
                    objects[i].crossBlob.pop_back();
                }

                /* 若畫面中原本物體移動，會連原本存在的位置也被判斷為移動，將會導致誤抓移動物件，因此以image difference做修正 */
                if (objects[i].trajectoryTime.size() % 5 == 4 ) {    //每五個畫面，才判斷一次是否變大中，其餘則是存當前blob大小
                    if (movingBlob.area() -  (objects[i].sumBlobs/4) > INTO_SCREEN_THRESHOLD && objects[i].overLap == 0) {   //代表進畫面中，物體持續變動，更新histgram

                        /* frame與前面frame平均相減並進行後處理 */
                        absdiff(frame_gray,frame_last,frame_diff);
                        threshold(frame_diff,frame_diffbin,20,255,THRESH_BINARY);
                        medianBlur(frame_diffbin, frame_diffmed, 5);
                        morphologyEx(frame_diffmed, frame_diffmorph, MORPH_CLOSE, element , Point( -1,-1 ) , MORPH_ITERATION);
                        std::vector < std::vector<cv::Point2i > > diffblobs;
                        FindBlobs(frame_diffmorph, diffblobs);

                        int dx_min,dy_min,dx_max,dy_max,t_dx_min,t_dy_min,t_dx_max,t_dy_max;
                        int dCheck = 0;
                        for(size_t j=0; j < diffblobs.size(); j++) {    //檢查每一個抓出的blob是否與moving blob重疊，是的話就加入等等的交集
                            t_dx_min = t_dx_max = diffblobs[j][0].x;
                            t_dy_min = t_dy_max = diffblobs[j][0].y;

                            for(size_t k=1; k < diffblobs[j].size(); k++) {         //the object's rect point
                                if (diffblobs[j][k].x < t_dx_min) t_dx_min = diffblobs[j][k].x;
                                if (diffblobs[j][k].y < t_dy_min) t_dy_min = diffblobs[j][k].y;
                                if (diffblobs[j][k].x > t_dx_max) t_dx_max = diffblobs[j][k].x;
                                if (diffblobs[j][k].y > t_dy_max) t_dy_max = diffblobs[j][k].y;
                            }
                            Rect jthRect(t_dx_min,t_dy_min,t_dx_max-t_dx_min+1,t_dy_max-t_dy_min+1);
                            if (IntersectionCheck(jthRect,movingBlob)) {
                                if (dCheck == 0) {  //第一個
                                    dx_min = t_dx_min;
                                    dy_min = t_dy_min;
                                    dx_max = t_dx_max;
                                    dy_max = t_dy_max;
                                    dCheck = 1;
                                }
                                else {
                                    if (t_dx_min < dx_min) dx_min = t_dx_min;
                                    if (t_dy_min < dy_min) dy_min = t_dy_min;
                                    if (t_dx_max > dx_max) dx_max = t_dx_max;
                                    if (t_dy_max > dy_max) dy_max = t_dy_max;
                                }
                            }
                        }
                        if (dCheck == 1) {
                            cout << "物體變大，將movingBlob改為image difference: [" << objects[i].id << "]" <<  endl;
                            movingBlob = Rect(dx_min,dy_min,dx_max-dx_min+1,dy_max-dy_min+1);
                            objects[i].trackWindow = movingBlob;
                        }
                        drawRect(frame_diffmorph,movingBlob,255,255,255,3);
                        //imshow("diff",frame_diffmorph);

                        cout << "** histogram更新: [" << objects[i].id << "]" << endl;

                        /** 測試中 先替換掉下面兩行 **/
                        calcWeightedHistogram(hsv, movingBlob, objects[i].hist, &tempmask);
                        //normalize(objects[i].hist, objects[i].hist, 0, 255, CV_MINMAX);     //如果hsize不是255就需要normalize
                        histgrams[objects[i].id-1] = objects[i].hist;


                        /* 由於histogram有變動，因此重新檢查是否與前幾秒消失的物件為同物件 （剛從遮蔽物出來）*/
                        if (objects[i].trajectoryTime.size() != 0) {
                            Point currentCenter = (objects[i].trackWindow.tl() + objects[i].trackWindow.br()) * 0.5;
                            int currentAvgArea = objects[i].sumArea/objects[i].trajectoryTime.size();
                            for (int k=0; k < oldObjects.size(); ++k) {
                                Point kCenter = oldObjects[k].trajectoryPoint.back();
                                int kAvgArea = oldObjects[k].sumArea/oldObjects[k].trajectoryTime.size();
                                if ( pow((kCenter.x-currentCenter.x),2)+pow((kCenter.y-currentCenter.y),2) < SAME_OBJECT_DISTANCE && abs(currentAvgArea-kAvgArea) < SAME_OBJECT_AREA_DIFFERENCE) {   //與消失物件之中心距離小於根號SAME_OBJECT_DISTANCE && 平均小相差值在合理範圍
                                    if (compareHist(oldObjects[k].hist, objects[i].hist, CV_COMP_CORREL) > SAME_HIST_THRESHOLD) {   //檢查histgram相似度，相似度夠高判斷為方才消失之物件(遮擋又出現)，繼承其過去資料繼續追蹤
                                        // 合併兩個物件的過去軌跡
                                        vector <int> newTimes;
                                        vector <Point> newPoints;
                                        vector <Rect> newRects;
                                        int oldSize = oldObjects[k].trajectoryTime.size();
                                        int m = 0;
                                        int n = 0;
                                        while(m < oldSize) {
                                            if (oldObjects[k].trajectoryTime[m] < objects[i].trajectoryTime[n]) {
                                                newTimes.push_back(oldObjects[k].trajectoryTime[m]);
                                                newPoints.push_back(oldObjects[k].trajectoryPoint[m]);
                                                newRects.push_back(oldObjects[k].trajectoryRect[m]);
                                                ++m;
                                            }
                                            else if (oldObjects[k].trajectoryTime[m] == objects[i].trajectoryTime[n]) {     //可能是(遮擋)斷開了
                                                newTimes.push_back(oldObjects[k].trajectoryTime[m]);
                                                newPoints.push_back( (oldObjects[k].trajectoryPoint[m]+objects[i].trajectoryPoint[n])*0.5 );
                                                Rect tpRect( (oldObjects[k].trajectoryRect[m].x+objects[i].trajectoryRect[n].x)/2, (oldObjects[k].trajectoryRect[m].y+objects[i].trajectoryRect[n].y)/2, (oldObjects[k].trajectoryRect[m].width+objects[i].trajectoryRect[n].width)/2, (oldObjects[k].trajectoryRect[m].height+objects[i].trajectoryRect[n].height)/2 );
                                                newRects.push_back(tpRect);
                                                ++m;
                                                ++n;
                                            }
                                            else{  //oldObjects[k].trajectoryTime[m] > objects[i].trajectoryTime[n] 應該是不會有這種情況
                                                newTimes.push_back(objects[i].trajectoryTime[n]);
                                                newPoints.push_back(objects[i].trajectoryPoint[n]);
                                                newRects.push_back(objects[i].trajectoryRect[n]);
                                                ++n;
                                            }
                                        }
                                        int objSize = objects[i].trajectoryTime.size();
                                        while (n < objSize) {
                                            newTimes.push_back(objects[i].trajectoryTime[n]);
                                            newPoints.push_back(objects[i].trajectoryPoint[n]);
                                            newRects.push_back(objects[i].trajectoryRect[n]);
                                            ++n;
                                        }

                                        objects[i].trajectoryTime = newTimes;
                                        objects[i].trajectoryPoint = newPoints;
                                        objects[i].trajectoryRect = newRects;
                                        cout << "histgram變動後發現為剛才消失之物件: [" << objects[i].id << "]" << " > [" << oldObjects[k].id << "]" << endl;
                                        objects[i].id = oldObjects[k].id;
                                        objects[i].sumArea = oldObjects[k].sumArea + objects[i].sumArea;
                                        objects[i].sumHeight = oldObjects[k].sumHeight + objects[i].sumHeight;
                                        objects[i].sumWidth = oldObjects[k].sumWidth + objects[i].sumWidth;
                                        objects[i].thief = oldObjects[k].thief;
                                        oldObjects.erase(oldObjects.begin()+k);

                                        break;
                                    }
                                }
                            }
                        }
                    }
                    objects[i].sumBlobs = 0;
                }
                else {  //0~4個frame 把moving blob累加
                    objects[i].sumBlobs += movingBlob.area();
                }

                objects[i].previousBlob = movingBlob;

                /* Object tracking: CAMShift */
                Mat trackingHist;
                objects[i].hist.copyTo(trackingHist);
                hist0To255(trackingHist);
                calcBackProject(&hsv, 1, channels, trackingHist, objects[i].backproj, phranges);
                objects[i].backproj &= mask;

                RotatedRect trackBox = CamShift(objects[i].backproj, objects[i].trackWindow,
                                                TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));

                if( objects[i].trackWindow.area() <= 1 )
                {
                    int cols = objects[i].backproj.cols, rows = objects[i].backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    objects[i].trackWindow = Rect(objects[i].trackWindow.x - r, objects[i].trackWindow.y - r,
                                                  objects[i].trackWindow.x + r, objects[i].trackWindow.y + r) &
                            Rect(0, 0, cols, rows);
                }
                /* Object tracking: CAMShift (end) */

                Rect trackBox_motion = movingBlob & trackBox.boundingRect();    //將tracking的結果與moving object detection的結果做合併
                if (trackBox_motion.area() < INTERSECTION_THRESHOLD) {    //可能是CAMSfhit追蹤跑掉了 將其重設回移動區域
                    cout << "reset: <" << objects[i].id << ">" << endl;
                    trackBox_motion = movingBlob;
                    objects[i].trackWindow = trackBox_motion;
                    /* 重設search window時順便更新histogram */
                    if (objects[i].overLap == 0) {
                        cout << "histgram updated: <" << objects[i].id << ">" << endl;
                        calcWeightedHistogram(hsv, trackBox_motion, objects[i].hist, &tempmask);
                        //normalize(objects[i].hist, objects[i].hist, 0, 255, CV_MINMAX);     //如果hsize不是255就需要normalize
                    }
                }
                if (objects[i].trackWindow.width > (trackBox_motion.width*TRACKWINDOW_THRESHOLD) || objects[i].trackWindow.height > (trackBox_motion.height*TRACKWINDOW_THRESHOLD)) {//CAMShift搜尋區域遠大於motion物件 重設
                    objects[i].trackWindow = Rect( movingBlob.x, movingBlob.y, (objects[i].trackWindow.width+trackBox_motion.width)/2, (objects[i].trackWindow.height+trackBox_motion.height)/2 );
                }

                if (objects[i].trajectoryPoint.size() > 0) { //若非物件出現的第一張的話則進行判斷是否移動過快(追蹤錯誤)
                    Point nCenter = (trackBox_motion.tl() + trackBox_motion.br()) * 0.5;
                    Point lCenter = objects[i].trajectoryPoint.back();
                    if (pow((nCenter.x-lCenter.x),2)+pow((nCenter.y-lCenter.y),2) > SPEED_LIMIT) {   //一秒移動太多，追錯了
                        objects[i].trackWindow = objects[i].trajectoryRect.back();
                        cout << "疑似追蹤錯誤: [" << objects[i].id << "]" << endl;
                        continue;   //捨棄這次結果
                    }
                }


                /* 判斷是否追到同一物件(位置、相似度) */
                int check = 0;
                if (objects[i].trajectoryTime.size() < 8) {
                    for (int k=0; k<i; ++k) {   //判斷其他物件有沒有跟此物件重疊，重疊的話則判斷是否是在追同一個物件，是的話把此物件砍了

                        if ( IntersectionCheck(trackBox_motion,objects[k].trackWindow)) {//兩個搜索區域重疊
                            if (compareHist(objects[k].hist, objects[i].hist, CV_COMP_CORREL) > SAME_HIST_THRESHOLD){// || coverCheck == 1) {//histgram相似度夠高，判斷為同個物件
                                cout << "判斷 [" << objects[i].id << "] 與 ["<< objects[k].id << "] 重合及相似度夠高判斷為同一物件追蹤，刪除[" << objects[i].id << "]" << endl;
                                //oldObjects.push_back(objects[i]);
                                objects.erase(objects.begin()+i);
                                check = 1;
                                i -= 1;
                                break;
                            }
                        }
                    }
                }
                if (check == 1) continue;   //物件i已被刪除

                RotatedRect trackBox2( Point2f(trackBox_motion.x+trackBox_motion.width/2, trackBox_motion.y+trackBox_motion.height/2), Size2f(trackBox_motion.width,trackBox_motion.height),0);
                trackBox2_vector.push_back(trackBox2);
                objects[i].trajectoryTime.push_back(pop_counter);
                objects[i].trajectoryPoint.push_back(trackBox2.center);
                objects[i].trajectoryRect.push_back(trackBox_motion);
                objects[i].sumArea += trackBox_motion.area();
                objects[i].sumHeight += trackBox_motion.height;
                objects[i].sumWidth += trackBox_motion.width;
                objects[i].overLap = 0;

                /* 前面判斷為可疑人物，畫在畫面上 */
                if (objects[i].thief == 1) {
                    ellipse( frame, trackBox2, Scalar(0,255,255), 3, CV_AA );
                    Point textOri = trackBox_motion.tl() - Point(0,30);
                    drawRect(frame, Rect(0,0,frameWidth,frameHeight), 0, 0, 255, 5);
                    putText(frame, "Suspicious human" , textOri, textFontFace , textFontScale, Scalar(0,255,255), textThickness);
                }
                else ellipse( frame, trackBox2, Scalar(0,0,255), 3, CV_AA );

                //cout << "[" << objects[i].id << "] center: " << trackBox2.center << " size: " << trackBox2.size << " sum: " << objects[i].sumWidth <<" , "<< objects[i].sumHeight<< endl;
                //cout << trackBox.center << " @ " << trackBox.size << endl;
                //cout << "1. trackwindow: " << objects[i].trackWindow.x << " " << objects[i].trackWindow.y << " " << objects[i].trackWindow.width << " " <<  objects[i].trackWindow.height << endl;
                //cout << "2. trackbox_motion: " << trackBox_motion.x << " " << trackBox_motion.y << " " << trackBox_motion.width << " " <<  trackBox_motion.height << endl << endl;
                //cout << "@ width: " << objects[i].trackWindow.width/trackBox_motion.width << "  height: " << objects[i].trackWindow.height/trackBox_motion.height << endl;

            }
            /* 仍存活的物件，開始進行特殊事件判斷、追蹤以及畫出追蹤區域等功能 (end)*/
        }
        /* 前面判斷為可疑車輛，畫在畫面上 */
        for(int k=0; k<carsNotGetOff.size(); ++k) {
            //drawRect(frame, carsNotGetOff[k], 255, 0, 255, 2);
            Point textOri = carsNotGetOff[k].tl() - Point(0,30);
            //putText(frame, "Suspicious car" , textOri, textFontFace , textFontScale, Scalar(0,255,255), textThickness);
            //drawRect(frame, Rect(0,0,frameWidth,frameHeight), 0, 0, 255, 5);
        }
        /* 前面判斷為特定區域被停車，畫在畫面上 */
        if (specialAlert > 0) {
            drawRect(frame, Rect(0,0,frameWidth,frameHeight), 0, 0, 255, 5);
            for (int k=0; k<specialAreas.size(); ++k) {
                Point textOri = specialAreas[k].tl() - Point(0,30);
                putText(frame, "Restricted parking" , textOri, textFontFace , textFontScale, Scalar(0,255,255), textThickness);
                drawRect(frame, specialAreas[k],255, 0, 255, 2);
            }
            --specialAlert;
        }
        /* 前面判斷為特定區域車子開出，畫在畫面上 */
        if (specialDriveAlive == 1) {
            //drawRect(frame, Rect(0,0,frameWidth,frameHeight), 0, 0, 255, 5);
            for (int k=0; k<specialAreas.size(); ++k) {
                Point textOri = specialAreas[k].tl() - Point(0,30);
                //putText(frame, "Restricted driving" , textOri, textFontFace , textFontScale, Scalar(0,255,255), textThickness);
                //drawRect(frame, specialAreas[k],255, 0, 255, 2);
            }
        }
        else {  //specialDriveAlive == 0
            specialDriveAlert = 0;
        }
        frame_gray.copyTo(frame_last);
        //imshow("frame",frame);
        //imshow("motion detection",frame_motion);
        //imshow("median filter",frame_median);
        //imshow("morphology",frame_morph);
        //imshow("MOG2FG",foreground);
        //imshow("MOG2BG",background);
        //imshow("connected components",output);
        //imshow("result",frame);
        //imshow("result1",frame1);
        //imshow("diff",frame_diffmorph);
        //cout << endl << "Frame: " << pop_counter << endl;

    }//end of if (pop_counter % NUM_FRAME_PROCESS == 0)
    for (int i=0; i < trackBox2_vector.size(); i++)
    {
        ellipse(frame, trackBox2_vector[i], Scalar(0,0,255), 3, CV_AA);
    }
}


/*======Thread Start==========*/
//======1.從Buffer中讀取Frame
//======2.接收顯示端的參數
//======3.對圖片做處理後傳送至顯示端
void ProcessThread::run()
{
    myLog->printLog(logAddMem("ProcessThread::run() enter1"));
    this->sleep(1);
    //第一次執行,Output Log
    cout << "Processing Thread Start"<< endl;
    string log_msg = "開始處理監視器影像串流";
    Log_for_display.push_back(log_msg);
    Log_Msg = "開始處理監視器影像串流";
    emit PrintMyLog(Log_Msg,0);
//    Output_Log("開始處理監視器影像串流");
    log_msg = "監視器IP:";log_msg+=cam_ip.toUtf8().constData();log_msg+=",Port:";log_msg+=to_string(cam_port);
//    Output_Log(log_msg);
    Log_for_display.push_back(log_msg);

    int BackgroundUpdate_couter = 0; // For Cost Testing
    double BackgroundUpdate_sum = 0; // For Cost Testing
    while(1)//外部迴圈 (目的:當重新與顯示端建立連線時,必須重頭來接收或傳送資料)
    {
        myLog->printLog(logAddMem("ProcessThread::run() enter2"));
        ////////////////////////////////
        // Stop thread if doStop=TRUE //
        ////////////////////////////////
        doStopMutex.lock();
        if(doStop)
        {
            doStop = false;
            doStopMutex.unlock();
            myLog->printLog(logAddMem("ProcessThread::run() enter3(doStop)"));
            break;
        }
        doStopMutex.unlock();
        /////////////////////////////////

        //        int64 tinit,tSpentTotal = 0;//測試花費時間用

        //傳送監視器的資訊至顯示端,若傳送失敗則重連並重傳
        if(Send_Camera_Info() == false)
            continue;

        //內部迴圈
        while(1)
        {
            myLog->printLog(logAddMem("ProcessThread::run() enter4"));
            ////////////////////////////////
            // Stop thread if doStop=TRUE //
            ////////////////////////////////
            doStopMutex.lock();
            if(doStop)
            {
                doStopMutex.unlock();
                myLog->printLog(logAddMem("ProcessThread::run() enter5(doStop)"));
                break;
            }
            doStopMutex.unlock();
            /////////////////////////////////

            //********************************************************************

            //*****************影像處理功能 接收參數*******************
            //*1.接收顯示端傳來的各個偵測事件開關                      *
            //*2.把每個事件佔一個位數,0表示關,1表示開,2表示參數要更新   *
            //********************************************************
            //*********************************接收EventCode***************************************

            myLog->printLog(logAddMem("ProcessThread::run() enter6"));
            int event_code_buf = 0;
            //檢查是否Timeout,是的話顯示錯誤訊息
            if(Recv_and_Check_Timeout(clientSock,(char *)&event_code_buf,sizeof(event_code_buf)))
            {
                //嘗試重新連線至顯示端直到成功連上
                myLog->printLog(logAddMem("ProcessThread::run() enter7"));
                reConnectToDisplay(server_ip,server_port);
                break;//回到外部迴圈,重頭來接收或傳送資料
            }
            Event_Detect_Code = event_code_buf;
            log_msg = "ProcessThread::run() enter8(EventCode:"; log_msg+=to_string(Event_Detect_Code); log_msg+=")";
            myLog->printLog(logAddMem(log_msg));
            //*********************************接收EventCode(End)***************************************

            //*********************** 從Buffer中Pop一張Frame *****************
            //等待Buffer有Frame, Pop 一張出來
            if(frame_queue->wait_and_pop(frame))
                pop_counter++;
            else//Stop Thread(重新連線呼叫WakeAllCondition Function導致)
                break;
            if(pop_counter==numeric_limits<int>::max() - 500)//避免overflow
                pop_counter = 0;
            myLog->printLog(logAddMem("ProcessThread::run() enter9"));
            //*********************** 從Buffer中Pop一張Frame (End) *************

            //resize(frame, frame, Size(1280, 720), 0, 0, INTER_CUBIC);//測試用，強制改變解析度

            //***************************根據開啟的功能 接收參數********************************************
            if(Event_Detect_Code >= 0)
            {   //【觸發事件1】偵測指定區域物體移動
                if(Event_Detect_Code % 10 == 2)//【觸發事件1】參數更新,接收參數
                {
                    string log_msg;
                    //Clear the ParkingLot rect vector
                    ParkingLot_ROI.clear();
                    ParkingLot_Display.clear();
                    //****************接收設定偵測物體區域的個數*********************
                    int ROI_size_buf[1];//ROI的個數
                    //檢查是否Timeout,是的話顯示錯誤訊息
                    if(Recv_and_Check_Timeout(clientSock,(char *)ROI_size_buf,sizeof(ROI_size_buf)))
                    {
                        //嘗試重新連線至顯示端直到成功連上
                        reConnectToDisplay(server_ip,server_port);
                        break;//回到外部迴圈,重頭來接收或傳送資料
                    }
                    cout << "The number of ROI:" << ROI_size_buf[0]<< endl;
                    //**************************************************************

                    //***********************接收設定偵測物體的區域(Rect)***********************************
                    int ROI_info_buf[4];
                    for(int i=0; i<ROI_size_buf[0]; i++)
                    {
                        //檢查是否Timeout,是的話顯示錯誤訊息
                        if(Recv_and_Check_Timeout(clientSock,(char *)ROI_info_buf,sizeof(ROI_info_buf)))
                        {
                            //嘗試重新連線至顯示端直到成功連上
                            reConnectToDisplay(server_ip,server_port);
                            break;//回到外部迴圈,重頭來接收或傳送資料
                        }

                        //目的：維持用原解析度的Frame來做影像處理
                        //方法：若解析度有調整過,ex:1280x720 -> 800x600 , 位置要調回原解析度對應的位置
                        if(fixed_resolution)
                        {
                            ROI_info_buf[0] *= width_ratio;
                            ROI_info_buf[1] *= height_ratio;
                            ROI_info_buf[2] *= width_ratio;
                            ROI_info_buf[3] *= height_ratio;
                        }
                        Rect temp(ROI_info_buf[0],ROI_info_buf[1],ROI_info_buf[2],ROI_info_buf[3]);//(x,y,width,height)
                        ParkingLot_ROI.push_back(temp);
                    }
                    //*******************************************************************************

                    //************************接收設定區域"顯示"的座標*****************************
                    int display_info_buf[2];
                    for(int i=0; i<ROI_size_buf[0] * 4; i++)//每格ROI有四個點來存放Display資訊
                    {
                        //檢查是否Timeout,是的話顯示錯誤訊息
                        if(Recv_and_Check_Timeout(clientSock,(char *)display_info_buf,sizeof(display_info_buf)))
                        {
                            //嘗試重新連線至顯示端直到成功連上
                            reConnectToDisplay(server_ip,server_port);
                            break;//回到外部迴圈,重頭來接收或傳送資料
                        }

                        //目的：維持用原解析度的Frame來做影像處理
                        //方法：若解析度有調整過,ex:1280x720 -> 800x600 , 位置要調回原解析度對應的位置
                        if(fixed_resolution)
                        {
                            display_info_buf[0] *= width_ratio;
                            display_info_buf[1] *= height_ratio;
                        }
                        Point temp(display_info_buf[0],display_info_buf[1]);//(x,y)
                        ParkingLot_Display.push_back(temp);
                    }
                    //**************************************************************************

                    //************************接收設定的靈敏度*****************************
                    int Sensitivity_buf[1];
                    //檢查是否Timeout,是的話顯示錯誤訊息
                    if(Recv_and_Check_Timeout(clientSock,(char *)Sensitivity_buf,sizeof(Sensitivity_buf)))
                    {
                        //嘗試重新連線至顯示端直到成功連上
                        reConnectToDisplay(server_ip,server_port);
                        break;//回到外部迴圈,重頭來接收或傳送資料
                    }

                    switch(Sensitivity_buf[0])
                    {//指定區域變化量超過全部的幾分之幾就警報
                    case 0 : detect_proportion = 0.25;break;//高 四分之一
                    case 1 : detect_proportion = 0.33;break;//中 三分之一
                    case 2 : detect_proportion = 0.5;break;//低 二分之一
                    default : break;
                    }
                    //********************************************************************

                    //===========Log訊息==============
                    log_msg = "[設定]指定區域個數:";
                    log_msg+= to_string(ROI_size_buf[0]);
                    log_msg+= "，靈敏度:";
                    switch(Sensitivity_buf[0])
                    {
                    case 0 : log_msg+= "高";break;
                    case 1 : log_msg+= "中";break;
                    case 2 : log_msg+= "低";break;
                    default : break;
                    }
//                    Output_Log(log_msg);
                    Log_for_display.push_back(log_msg);
                    //===========Log訊息 End==============

                    /* Delete the pointer */
                    if(detectObjectInOutROI){
                        delete [] detectObjectInOutROI;
                        detectObjectInOutROI = NULL;
                    }

                    /* 建立指定區域偵測物件 */
                    detectObjectInOutROI = new DetectAssignArea[ParkingLot_ROI.size()];

                    // 設定差異數目門檻值
                    for(int n=0; n<ParkingLot_ROI.size(); n++){
                        detectObjectInOutROI[n].setDiffThreshold((double)((ParkingLot_ROI[n].width) * (ParkingLot_ROI[n].height)) * detect_proportion);
                    }
                    cout << "偵測區域參數初始化完成"<< endl;
                    //======== 指定區域偵測 =======
                    processingMutex.lock();
                    if(pop_counter % NUM_FRAME_PROCESS == 0)//每[NUM_FRAME_PROCESS]張Frame處理一次
                    {
                        ROI_Detect_Objects(frame);//指定區域偵測
                        BackgroundUpdate(&frame_bg,&frame_ref,frame_gray,ROIs);//全部背景更新
                        cout << "BackGround All Update"<< endl;
                    }
                    Draw_ROI_Detect(frame);//畫出所有超過ThreshHold的指定區域(Debug)
                    processingMutex.unlock();
                }
                //【觸發事件1】沒有更新,但有啟動偵測指定區域物體移動
                else if(Event_Detect_Code % 10 == 1){
                    //======== 指定區域偵測 =======
                    processingMutex.lock();
                    if(pop_counter % NUM_FRAME_PROCESS == 0)//每[NUM_FRAME_PROCESS]張Frame處理一次
                    {
                        ROI_Detect_Objects(frame);//指定區域偵測
                        time_t start, end;
                        time(&start);
                        BackgroundUpdate(&frame_bg,&frame_ref,frame_gray,ROIs);//全部背景更新
                        time(&end);
                        cout << "BackGround All Update"<< endl;
                        BackgroundUpdate_couter++;
                        double seconds = difftime(end,start);
                        BackgroundUpdate_sum += seconds;
                        cout << "BackgroundUpdate average cost : " << BackgroundUpdate_sum/BackgroundUpdate_couter << "s" <<endl;
                    }
                    Draw_ROI_Detect(frame);//畫出所有超過ThreshHold的指定區域(Debug)
                    processingMutex.unlock();
                }
                //【觸發事件1】沒有啟動偵測指定區域物體移動
                else{
                }
                //【觸發事件2】偵測手機事件, 參數更新, 接收偵測手機事件參數
                if(Event_Detect_Code % 100 >= 20)
                {
                    myLog->printLog(logAddMem("ProcessThread::run() detectPhone Update1"));
                    //************************ 接收設定偵測手機的靈敏度 *****************************
                    int Sensitivity_buf[1];
                    //檢查是否Timeout,是的話顯示錯誤訊息
                    if(Recv_and_Check_Timeout(clientSock,(char *)Sensitivity_buf,sizeof(Sensitivity_buf)))
                    {
                        //嘗試重新連線至顯示端直到成功連上
                        reConnectToDisplay(server_ip,server_port);
                        break;//回到外部迴圈,重頭來接收或傳送資料
                    }
                    this->detect_phone_sensitivity = Sensitivity_buf[0];
                    //********************************************************************
                    log_msg="ProcessThread::run() detectPhone Update2(Sensitivity:";
                    log_msg+= to_string(this->detect_phone_sensitivity); log_msg+=")";
                    myLog->printLog(logAddMem(log_msg));
                    //複製一份原始的Frame給手機行為辨識
                    frame.copyTo(cellphone_frame);
                    //New一個Phone Thread,丟入detect_phone_continuity參數
                    if(!phone_thread){
                        myLog->printLog(logAddMem("ProcessThread::run() detectPhone Update3(New phone thread)"));
                        phone_thread = new cellphone_detection(&cellphone_frame,&this->detect_phone_sensitivity);
                    }
                    phone_thread->start(HighPriority);//執行手機偵測的Phone Thread
                    warningmessage phoneWarning;
                    phoneAlert = phoneWarning.getMessage();//是否有觸發警報
                    myLog->printLog(logAddMem("ProcessThread::run() detectPhone Update4(End)"));
                }
                //【觸發事件2】沒有更新,但有啟動偵測手機事件
                else if(Event_Detect_Code % 100 >= 10){
                    myLog->printLog(logAddMem("ProcessThread::run() detectPhone On1"));
                    //複製一份原始的Frame給手機行為辨識
                    frame.copyTo(cellphone_frame);
                    phone_thread->start(HighPriority);
                    warningmessage phoneWarning;
                    phoneAlert = phoneWarning.getMessage();//是否有觸發警報
                    myLog->printLog(logAddMem("ProcessThread::run() detectPhone On2(End)"));
                }
                //【觸發事件2】沒有啟動偵測手機事件
                else{
                    myLog->printLog(logAddMem("ProcessThread::run() detectPhone Off1"));
                    //如果之前有啟動過偵測手機thread, 則停止它
                    if(phone_thread)
                    {
                        myLog->printLog(logAddMem("ProcessThread::run() detectPhone Off2(Stop phone thread)"));
                        //停止上一個執行的Phone Thread
                        phone_thread->stop();
                        phone_thread->wait();
                        phone_thread = NULL;
                    }
                    myLog->printLog(logAddMem("ProcessThread::run() detectPhone Off3(End)"));
                }
                //【觸發事件3】SMTP參數更新
                if(Event_Detect_Code % 1000 >= 200){
                    qDebug() << "========= SMTP =========" ;
                    int smtpServerLen, smtpSenderLen, smtpPasswordLen, smtpReceiverLen;
                    //接收參數1：SMTP Server(字串長度和內容)
                    if(Recv_and_Check_Timeout(clientSock,(char *)&smtpServerLen, sizeof(int))){
                        reConnectToDisplay(server_ip,server_port);//嘗試重新連線至顯示端直到成功連上
                        break;//回到外部迴圈,重頭來接收或傳送資料
                    }
                    qDebug() << "smtpServerLen:" << smtpServerLen;
                    char smtpServerBuf[smtpServerLen];
                    if(Recv_and_Check_Timeout(clientSock,(char *)smtpServerBuf, smtpServerLen+1)){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    else
                        this->smtpServer = smtpServerBuf;
                    qDebug() << "this->smtpServer:" << QString::fromStdString(this->smtpServer);
                    //接收參數2：SMTP Port(內容)
                    int smtpPortBuf;
                    if(Recv_and_Check_Timeout(clientSock,(char *)&smtpPortBuf, sizeof(int))){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    else
                        this->smtpPort = smtpPortBuf;
                    qDebug() << "this->smtpPort:" << QString::number(smtpPort);
                    //接收參數3：SMTP Sender(字串長度和內容)
                    if(Recv_and_Check_Timeout(clientSock,(char *)&smtpSenderLen, sizeof(int))){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    qDebug() << "smtpSenderLen:" << smtpSenderLen;
                    char smtpSenderBuf[smtpSenderLen];
                    if(Recv_and_Check_Timeout(clientSock,(char *)smtpSenderBuf, smtpSenderLen+1)){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    else
                        this->smtpSender = smtpSenderBuf;
                    qDebug() << "this->smtpSender:" << QString::fromStdString(this->smtpSender);
                    //接收參數4：SMTP Sender's Password(字串長度和內容)
                    if(Recv_and_Check_Timeout(clientSock,(char *)&smtpPasswordLen, sizeof(int))){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    qDebug() << "smtpPasswordLen:" << smtpPasswordLen;
                    char smtpPasswordBuf[smtpPasswordLen];
                    if(Recv_and_Check_Timeout(clientSock,(char *)smtpPasswordBuf, smtpPasswordLen+1)){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    else
                        this->smtpPassword = smtpPasswordBuf;
                    //接收參數5：SMTP Receiver(字串長度和內容)
                    if(Recv_and_Check_Timeout(clientSock,(char *)&smtpReceiverLen, sizeof(int))){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    qDebug() << "smtpReceiverLen:" << smtpReceiverLen;
                    char smtpReceiverBuf[smtpReceiverLen];
                    if(Recv_and_Check_Timeout(clientSock,(char *)smtpReceiverBuf, smtpReceiverLen+1)){
                        reConnectToDisplay(server_ip,server_port);
                        break;
                    }
                    else
                        this->smtpReceiver = smtpReceiverBuf;
                    qDebug() << "this->smtpReceiver:" << QString::fromStdString(this->smtpReceiver);
                    //複製一張Frame給SMTP寄送用
                    frame.copyTo(send_frame);
                }
                //【觸發事件3】SMTP有啟動
                else if(Event_Detect_Code % 1000 >= 100){
                    //複製一張Frame給SMTP寄送用
                    frame.copyTo(send_frame);
                }
                //【觸發事件3】SMTP沒有啟動
                else{
                }
            }//End of (if(Event_Detect_Code >= 0))
            myLog->printLog(logAddMem("ProcessThread::run() enter10"));

            //======================傳送影像串流Data至顯示端=========================
            //Step 1-1：影像縮小
            if(fixed_resolution){//若解析度超過800x600 , 將送出去的frame解析度降為800x600
                myLog->printLog(logAddMem("ProcessThread::run() enter10-1"));
                resize(frame, frame, Size(800, 600), 0, 0, INTER_CUBIC);
                myLog->printLog(logAddMem("ProcessThread::run() enter10-2"));
            }
            //Step 1-2：影像壓縮
            frameBuff.clear();
             myLog->printLog(logAddMem("ProcessThread::run() enter10-3"));
            cv::imencode(".jpg", frame, frameBuff, params );
             myLog->printLog(logAddMem("ProcessThread::run() enter10-4"));
            int compressedImgSize = frameBuff.size();
            log_msg="ProcessThread::run() enter11(compressedImgSize:"; log_msg+=to_string(compressedImgSize); log_msg+=")";
            myLog->printLog(logAddMem(log_msg));
            // Step 2：傳送壓縮影像資料長度
            if(Send_and_Check_Timeout(clientSock,(char *)&compressedImgSize, sizeof(int))){
                reConnectToDisplay(server_ip,server_port);//嘗試重新連線至顯示端直到成功連上
                break;//回到外部迴圈,重頭來接收或傳送資料
            }
            myLog->printLog(logAddMem("ProcessThread::run() enter12"));
            //Step 3：傳送壓縮影像資料
            //檢查是否Timeout,是的話顯示錯誤訊息
            if(Send_and_Check_Timeout(clientSock,reinterpret_cast<char*>(frameBuff.data()),compressedImgSize))
            {
                //嘗試重新連線至顯示端直到成功連上
                reConnectToDisplay(server_ip,server_port);
                break;//回到外部迴圈,重頭來接收或傳送資料
            }
            myLog->printLog(logAddMem("ProcessThread::run() enter13"));
            //======================================================================


            //=================== 有沒有觸發事件警告 ===================
            int alert_buf[1];
            if(roiAlert || phoneAlert){//是否有觸發警報
                alert_buf[0] = 1;
                if(Event_Detect_Code % 1000 >= 100)//若有開啟SMTP功能
                    doSendEmailByEvent(send_frame,8);//根據事件發送Email(附上當前圖片), 設定寄信間隔時間
                if(phoneAlert){
                    warningmessage phoneWarning;
                    phoneWarning.setWarning(false);//將偵測手機警報Flag改為False,避免一直觸發警報
                }
            }
            else{
                alert_buf[0] = 0;
            }
            // ===== 傳送警告訊息至顯示端 =====
            if(Send_and_Check_Timeout(clientSock,(char *)alert_buf,sizeof(alert_buf)))
            {
                //嘗試重新連線至顯示端直到成功連上
                reConnectToDisplay(server_ip,server_port);
                break;//回到外部迴圈,重頭來接收或傳送資料
            }
           myLog->printLog(logAddMem("ProcessThread::run() enter14"));

            //========================================================

            //===========================傳送Log訊息=================================
            int num_log_buf[1] = {Log_for_display.size()};//Log訊息數量
            //檢查是否Timeout,是的話顯示錯誤訊息
            if(Send_and_Check_Timeout(clientSock,(char *)num_log_buf,sizeof(num_log_buf)))
            {
                //嘗試重新連線至顯示端直到成功連上
                reConnectToDisplay(server_ip,server_port);
                break;//回到外部迴圈,重頭來接收或傳送資料
            }
            myLog->printLog(logAddMem("ProcessThread::run() enter15"));
            for(int i=0; i<Log_for_display.size(); i++)
            {
                //******************傳送每個Log訊息的字串長度********************
                int log_info_len = htonl(Log_for_display[i].size());
                //檢查是否Timeout,是的話顯示錯誤訊息
                if(Send_and_Check_Timeout(clientSock,(char *)&log_info_len,sizeof(int)))
                {
                    //嘗試重新連線至顯示端直到成功連上
                    reConnectToDisplay(server_ip,server_port);
                    break;//回到外部迴圈,重頭來接收或傳送資料
                }
                myLog->printLog(logAddMem("ProcessThread::run() enter16"));
                //**************************************************************
                //********************傳送Log訊息*******************************
                char log_info_buf[Log_for_display[i].size()];
                strcpy(log_info_buf, Log_for_display[i].c_str());
                //檢查是否Timeout,是的話顯示錯誤訊息
                if(Send_and_Check_Timeout(clientSock,(char *)log_info_buf,sizeof(log_info_buf)))
                {
                    //嘗試重新連線至顯示端直到成功連上
                    reConnectToDisplay(server_ip,server_port);
                    break;//回到外部迴圈,重頭來接收或傳送資料
                }
                myLog->printLog(logAddMem("ProcessThread::run() enter17"));
                //**************************************************************
            }
            //清空Log訊息的Buffer
            Log_for_display.clear();
        }//end of inner while loop
    }//end of outer while loop
    myLog->printLog(logAddMem("ProcessThread::run() enter18(Break the Loop)"));
    //停止執行中的Phone Thread
    if(phone_thread)
    {
        myLog->printLog(logAddMem("ProcessThread::run() enter19(Stop phone thread)"));
        phone_thread->stop();
        phone_thread->wait();
        phone_thread = NULL;
    }
    myLog->printLog(logAddMem("ProcessThread::run() enter20"));
    //清空Log訊息的Buffer
    Log_for_display.clear();
    //Close Sock
    closesocket(clientSock);
    //Clean the Win Socket Network
    WSACleanup();
    myLog->printLog(logAddMem("ProcessThread::run() enter21(End)"));
}

bool ProcessThread::Send_Camera_Info()
{
    string log_msg="";
    myLog->printLog(logAddMem("ProcessThread::Send_Camera_Info() enter1"));
    //====================== 傳送Frame的 解析度 和 FPS ========================
    int frame_info_buf[2];
    if(fixed_resolution)//若超出 800 x 600 解析度 , 則固定解析度 800 x 600
    {
        frame_info_buf[0] = 800;
        frame_info_buf[1] = 600;
    }
    else//沒超過則維持原解析度
    {
        frame_info_buf[0] = frameWidth;
        frame_info_buf[1] = frameHeight;
    }
    //傳送監視器解析度
    if(Send_and_Check_Timeout(clientSock,(char *)frame_info_buf, sizeof(frame_info_buf)))
    {
        //嘗試重新連線至顯示端直到成功連上
        reConnectToDisplay(server_ip,server_port);
        myLog->printLog(logAddMem("ProcessThread::Send_Camera_Info() exit1(reConnectToDisplay)"));
        return false;//重頭傳送資料
    }
    log_msg = "ProcessThread::Send_Camera_Info() enter2(frameWidth:";
    log_msg+= to_string(frame_info_buf[0]); log_msg+=" frameHeight:"; log_msg+=to_string(frame_info_buf[1]); log_msg+=")";
    myLog->printLog(logAddMem(log_msg));
    //傳送監視器FPS
    int camera_fps_buf[1] = {Cap_FPS};
    if(Send_and_Check_Timeout(clientSock,(char *)camera_fps_buf, sizeof(camera_fps_buf)))
    {
        //嘗試重新連線至顯示端直到成功連上
        reConnectToDisplay(server_ip,server_port);
        myLog->printLog(logAddMem("ProcessThread::Send_Camera_Info() exit2(reConnectToDisplay)"));
        return false;//重頭傳送資料
    }
    log_msg = "ProcessThread::Send_Camera_Info() enter3(FPS:"; log_msg+=to_string(Cap_FPS); log_msg+=")";
    myLog->printLog(logAddMem(log_msg));
    //==================================================================

    //****************** Send 監視器地點 字串長度 和 內容 *******************
    int location_length_buf  = strlen(cam_location);
    //    cout << "Camera's location length : " << location_length_buf<< endl;
    //    cout << "Camera's location : " << cam_location<< endl;
    //    cout << "sizeof(location_length_buf) : " << sizeof(location_length_buf)<< endl;
    //    cout << "strlen(cam_location) : " << strlen(cam_location)<< endl;
    //傳送地點字串長度
    if(Send_and_Check_Timeout(clientSock,(char *)&location_length_buf, sizeof(int)))
    {
        //嘗試重新連線至顯示端直到成功連上
        reConnectToDisplay(server_ip,server_port);
        myLog->printLog(logAddMem("ProcessThread::Send_Camera_Info() exit3(reConnectToDisplay)"));
        return false;//重頭傳送資料
    }
    myLog->printLog(logAddMem("ProcessThread::Send_Camera_Info() enter4"));
    //傳送地點內容
    if(Send_and_Check_Timeout(clientSock, cam_location, strlen(cam_location)+1))
    {
        //嘗試重新連線至顯示端直到成功連上
        reConnectToDisplay(server_ip,server_port);
        myLog->printLog(logAddMem("ProcessThread::Send_Camera_Info() exit4(reConnectToDisplay)"));
        return false;//重頭傳送資料
    }
    log_msg = "ProcessThread::Send_Camera_Info() exit5(cam_location:"; log_msg+=cam_location; log_msg+=")";
    myLog->printLog(logAddMem(log_msg));
    //******************************************************************
    return true;
}
//檢查是否Timeout,是的話顯示錯誤訊息
bool ProcessThread::Recv_and_Check_Timeout(SOCKET connection, char *buf, int buf_len)
{
    myLog->printLog(logAddMem("ProcessThread::Recv_and_Check_Timeout() enter"));
    //recv bytes
    int bytes = 0;
    //setting the send and receive timeout
    int timeout = 10000; //in milliseconds.
    setsockopt(connection,SOL_SOCKET,SO_RCVTIMEO,(const char *)&timeout,sizeof(timeout));
    bytes = recv(connection, (char *)buf,buf_len, 0);
    //cout << "Recv Bytes:" << bytes;
    if(bytes == SOCKET_ERROR)
    {
        //****************Output 錯誤訊息*****************
        printf("%s %s recv() error %ld.\n",get_current_time().c_str(),this->cam_location, WSAGetLastError());
        string log_msg = "[斷線]處理端無法從顯示端接收資料! "; log_msg+=WSAGetLastError();
        Log_for_display.push_back(log_msg);
        Log_Msg = "[斷線]處理端無法從顯示端接收資料! "; Log_Msg+=WSAGetLastError();
        emit PrintMyLog(Log_Msg,2);
        myLog->printLog(logAddMem("ProcessThread::Recv_and_Check_Timeout() exit1(Error)"));
        return true;
    }
    else if(bytes == 0)
    {
        //****************Output 訊息*****************
        printf("%s %s Recv bytes = 0 error %ld.\n",get_current_time().c_str(),this->cam_location, WSAGetLastError());
        string log_msg = "[斷線]連線關閉! "; log_msg+=WSAGetLastError();
        Log_for_display.push_back(log_msg);
        Log_Msg = "[斷線]連線關閉! "; Log_Msg+=WSAGetLastError();
        emit PrintMyLog(Log_Msg,2);
        myLog->printLog(logAddMem("ProcessThread::Recv_and_Check_Timeout() exit2(Error)"));
        return true;
    }
    else{
        myLog->printLog(logAddMem("ProcessThread::Recv_and_Check_Timeout() exit3(Succeed)"));
        return false;
    }
}

bool ProcessThread::Send_and_Check_Timeout(SOCKET connection, char *buf, int buf_len)
{
    myLog->printLog(logAddMem("ProcessThread::Send_and_Check_Timeout() enter"));
    //recv bytes
    int bytes = 0;
    //setting the send and receive timeout
    int timeout = 10000; //in milliseconds.
    setsockopt(connection,SOL_SOCKET,SO_SNDTIMEO,(const char *)&timeout,sizeof(timeout));
    bytes = send(connection, (char *)buf,buf_len, 0);
    //    cout << "Send Bytes:" << bytes;
    if(bytes == SOCKET_ERROR)
    {
        //****************Output 錯誤訊息*****************
        printf("%s %s send() error %ld.\n",get_current_time().c_str(),this->cam_location, WSAGetLastError());
        string log_msg = "[斷線]處理端無法傳送資料至顯示端!";
        Log_for_display.push_back(log_msg);
        Log_Msg = "[斷線]處理端無法傳送資料至顯示端!";
        emit PrintMyLog(Log_Msg,2);
        myLog->printLog(logAddMem("ProcessThread::Send_and_Check_Timeout() exit1(Error)"));
        return true;
    }
    else if(bytes == 0)
    {
        //****************Output 訊息*****************
        printf("%s %s Send bytes = 0 error %ld.\n",get_current_time().c_str(),this->cam_location, WSAGetLastError());
        string log_msg = "[斷線]連線關閉! "; log_msg+=WSAGetLastError();
        Log_for_display.push_back(log_msg);
        Log_Msg = "[斷線]連線關閉! "; Log_Msg+=WSAGetLastError();
        emit PrintMyLog(Log_Msg,2);
        myLog->printLog(logAddMem("ProcessThread::Send_and_Check_Timeout() exit2(Error)"));
        return true;
    }
    else{
        myLog->printLog(logAddMem("ProcessThread::Send_and_Check_Timeout() exit3(Succeed)"));
        return false;
    }
}
void ProcessThread::stop()
{
    QMutexLocker locker(&doStopMutex);
    myLog->printLog(logAddMem("ProcessThread::stop() enter"));
    doStop=true;
    myLog->printLog(logAddMem("ProcessThread::stop() exit"));
}


//Output訊息至今天日期的Log檔
//void ProcessThread::Output_Log(const string messages)
//{
//    ofstream mylog;
//    //依照日期打開對應日期的txt檔
////    log_filename = "Log "; log_filename += get_current_date(); log_filename += ".txt";
//    log_filename = "output.log";
//    myLog->open(log_filename,ios::out | ios::app);
//    if(!mylog)
//    {
//        cout << "[Error] Fail to open MyLog"<< endl;
//        exit(1);
//    }
//    else
//    {
//        get_current_time(mylog);
//        mylog << "  " << messages << "(" << this << ")"<< endl;
//        myLog->close();
//    }
//}

string ProcessThread::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}

string ProcessThread::get_current_time()
{
    string current_time;
    time_t cap_t = time(0);   // get time now
    struct tm * now = localtime( & cap_t );
    current_time  = to_string(now->tm_year + 1900);
    current_time += "/"; current_time += to_string(now->tm_mon + 1);
    current_time += "/"; current_time += to_string(now->tm_mday);
    current_time += " "; current_time +=to_string(now->tm_hour);
    current_time += ":"; current_time +=to_string(now->tm_min);
    current_time += ":"; current_time +=to_string(now->tm_sec);
    return current_time;
}

/***** ImgProcess Functions *****/
/** 輸出畫面大小 **/
void ProcessThread::test()
{
    cout << frameWidth << "  " << frameHeight << endl;
    return;
}

/** RGB轉灰階 **/
Mat ProcessThread::Grayscale(Mat original)
{
    //cout << "ProcessThread::Grayscale";
    Mat gray;
    cvtColor(original, gray, CV_BGR2GRAY);
    return gray;
}

/** 初始化背景 **/
bool ProcessThread::BackgroundInitialize(VideoCapture &cap, Mat &bg, Mat &ref, Mat &last,vector <Rect> areas)
{
    myLog->printLog(logAddMem("ProcessThread::BackgroundInitialize() enter"));
    //cout << "ProcessThread::BackgroundInitialize";
    /* 取前三十張做平均 */
    const int START_AVERAGE = 30;
    Mat frame, frame_gray, *buffer = new Mat[START_AVERAGE];
    Size dsize = Size(frameWidth, frameHeight);
    for (int i = START_AVERAGE-1; i >= 0; --i)  {
        cap >> frame;
        if (frame.empty())  {
            cerr << "Not enough frames\n";
            return false;
        }
        resize(frame, frame, dsize);
        frame_gray = Grayscale(frame);
        frame_gray.copyTo(buffer[i]);
    }
    buffer[0].copyTo(bg);
    buffer[0].copyTo(ref);
    //跑每一個pixel，將該pixel三十張相加取平均
    for (int rc = 0; rc < areas.size(); ++rc) {
        int x = areas[rc].x;
        int y = areas[rc].y;
        int width = areas[rc].width;
        int height = areas[rc].height;
        int x_bound = x + width;
        int y_bound = y + height;
        for(int i = y; i < y_bound; i++) {
            uchar* Bi = bg.ptr<uchar>(i);
            uchar* Ri = ref.ptr<uchar>(i);
            for(int j = x; j < x_bound; j++) {
                int counter = 0;
                for (int k = 0; k < START_AVERAGE; ++k) {
                    const uchar* Bufi = buffer[k].ptr<uchar>(i);
                    counter += Bufi[j];
                }
                Bi[j] = Ri[j] = counter/START_AVERAGE;
            }
        }
    }
    buffer[START_AVERAGE-1].copyTo(last);
    delete [] buffer;
    myLog->printLog(logAddMem("ProcessThread::BackgroundInitialize() exit"));
    return true;
}

/** 背景更新 **/
void ProcessThread::BackgroundUpdate(Mat* bg, Mat* ref, Mat gray, vector <Rect> areas)
{
    myLog->printLog(logAddMem("ProcessThread::BackgroundUpdate() enter"));
    gray = Grayscale(frame);//轉灰階
    //cout << "ProcessThread::BackgroundUpdate";
    for (int rc = 0; rc < areas.size(); ++rc) {
        int x = areas[rc].x;
        int y = areas[rc].y;
        int width = areas[rc].width;
        int height = areas[rc].height;
        int x_bound = x + width;
        int y_bound = y + height;
        int sum = 0;
        //跑每一個pixel，若較大就+1，反之則-1
        for(int i = y; i < y_bound; i++) {
            const uchar* Gi = gray.ptr<uchar>(i);
            uchar* Ri = ref->ptr<uchar>(i);
            uchar* Bi = bg->ptr<uchar>(i);
            for(int j = x; j < x_bound; j++) {
                if (Ri[j] < Gi[j]) {    //Reference frame generation
                    Ri[j] += min(Gi[j]-Ri[j] ,1);
                }
                else {
                    Ri[j] -= min(Ri[j]-Gi[j] ,1);
                }
                if (Ri[j] == Gi[j]) {   //Temporal match
                    if (Bi[j] < Ri[j]) {    //Background modification
                        Bi[j] += min(Ri[j]-Bi[j] ,1);
                        Ri[j] -= min(Ri[j]-Bi[j] ,1);//Reference frame modification
                    }
                    else {
                        Bi[j] -= min(Bi[j]-Ri[j] ,1);
                        Ri[j] += min(Bi[j]-Ri[j] ,1);//Reference frame modification
                    }
                }
            }
        }
    }
    myLog->printLog(logAddMem("ProcessThread::BackgroundUpdate() exit"));
}

/** 比較兩frame差異 (目前已無用到) **/
int ProcessThread::FrameCompare(Mat frame,Mat background,CvRect area)
{
    //cout << "ProcessThread::FrameCompare";
    int x = area.x;
    int y = area.y;
    int width = area.width;
    int height = area.height;
    int x_bound = x + width;
    int y_bound = y + height;
    int sum = 0;
    //跑每一個pixel，背景與當前frame相減，大於THRESHOLD則sum+1
    for(int i = y; i < y_bound; i++) {
        const uchar* Fi = frame.ptr<uchar>(i);
        const uchar* Bi = background.ptr<uchar>(i);
        for(int j = x; j < x_bound; j++) {
            if ( abs(Fi[j]-Bi[j]) > MOVING_PIXEL_THRESHOLD ) {
                ++sum;
            }
        }
    }

    if ( (1.0 * sum / (width * height)) > MOVING_OBJECT_THRESHOLD ) return 1;

    return 0;

}

/** 找出移動點 0=背景 255=moving pixels **/
int ProcessThread::MotionMask(Mat frame,Mat background, vector <Rect> areas)
{
    //cout << "ProcessThread::MotionMask";
    int sum = 0;
    int diff = 0;
    int histogram[256] = {0};
    for (int rc = 0; rc < areas.size(); ++rc) {
        int x = areas[rc].x;
        int y = areas[rc].y;
        int width = areas[rc].width;
        int height = areas[rc].height;
        int x_bound = x + width;
        int y_bound = y + height;

        //計算absolute difference (背景跟當前frame)
        for(int i = y; i < y_bound; i++) {
            const uchar* Fi = frame.ptr<uchar>(i);
            const uchar* Bi = background.ptr<uchar>(i);
            for(int j = x; j < x_bound; j++) {
                diff = abs(Fi[j] - Bi[j]);
                ++histogram[diff];
            }
        }
    }
    //計算要帶入cauchy distribution的a1和a2兩個參數
    int num_sum = 0;
    long long level_num_sum = 0;
    for (int i = 0;i < SCALE_PARAMETER; ++i) {
        num_sum += histogram[i];
        level_num_sum += i * histogram[i];
    }
    double a1 = 1.0 * level_num_sum / num_sum;
    num_sum = 0;
    level_num_sum = 0;
    for (int i = SCALE_PARAMETER+1; i < 256; ++i) {
        num_sum += histogram[i];
        level_num_sum += i * histogram[i];
    }
    double a2 = 1.0 * level_num_sum / num_sum;

    //跑每一個pixel 利用公式判斷該pixel屬於前景或背景
    for (int rc = 0; rc < areas.size(); ++rc) {
        int x = areas[rc].x;
        int y = areas[rc].y;
        int width = areas[rc].width;
        int height = areas[rc].height;
        int x_bound = x + width;
        int y_bound = y + height;

        for(int i = y; i < y_bound; i++) {
            uchar* Fi = frame.ptr<uchar>(i);
            const uchar* Bi = background.ptr<uchar>(i);
            for(int j = x; j < x_bound; j++) {
                diff = abs(Fi[j] - Bi[j]);
                if (pow((diff - a2),2) < pow((diff - a1),2)) { //f1 < f2 (1/a1 < 1/a2 => a2 < a1)
                    ++sum;
                    Fi[j] = 255;
                }
                else {
                    Fi[j] = 0;
                }
            }
        }
    }
    for(int i = 0; i < frameHeight; i++) {
        uchar* Fi = frame.ptr<uchar>(i);
        for(int j = 0; j < frameWidth; j++) {
            if (Fi[j] < 255)
                Fi[j] = 0;
        }
    }

    //if ( (1.0 * sum / (width * height)) > MOVING_OBJECT_THRESHOLD ) return 1;

    return 0;

}

/** 記錄要的frame (目前已無用到) **/
void ProcessThread::FrameRecord(string name, int num)
{
    //cout << "ProcessThread::FrameRecord";
    fstream fp;
    fp.open(name.c_str(), std::fstream::out | std::fstream::app);
    fp << num - NUM_FRAME_SAVE << endl;

    fp.close();
    return;
}

/** 找出連通物件(方法一) (目前已無用到) **/
void ProcessThread::ConnectedComponents(IplImage *pImg,int areaThreshold)
{
    //cout << "ProcessThread::ConnectedComponents";
    CvMemStorage * storage = cvCreateMemStorage(0);
    CvSeq * contour = 0;

    cvFindContours( pImg, storage, &contour, sizeof(CvContour),
                    CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE , cvPoint(0,0));

    int i=0;
    for(;contour;contour = contour->h_next)
    {
        CvRect aRect = cvBoundingRect( contour, 0 );
        if((aRect.width*aRect.height)>areaThreshold)
        {
            cvRectangle( pImg, cvPoint(aRect.x,aRect.y),cvPoint((aRect.x+aRect.width),
                                                                (aRect.y+aRect.height)),CV_RGB(255,0,0), 3 );
        }
    }
}

/** 找出連通物件(方法二) (目前已無用到) **/
void ProcessThread::icvprCcaByTwoPass(const cv::Mat& _binImg, cv::Mat& _lableImg)
{
    //cout << "ProcessThread::icvprCcaByTwoPass";
    // connected component analysis (4-component)
    // use two-pass algorithm
    // 1. first pass: label each foreground pixel with a label
    // 2. second pass: visit each labeled pixel and merge neighbor labels
    //
    // foreground pixel: _binImg(x,y) = 1
    // background pixel: _binImg(x,y) = 0


    if (_binImg.empty() ||
            _binImg.type() != CV_8UC1)
    {
        return ;
    }

    // 1. first pass

    _lableImg.release() ;
    _binImg.convertTo(_lableImg, CV_32SC1) ;

    int label = 1 ;  // start by 2
    std::vector<int> labelSet ;
    labelSet.push_back(0) ;   // background: 0
    labelSet.push_back(1) ;   // foreground: 1

    int rows = _binImg.rows - 1 ;
    int cols = _binImg.cols - 1 ;
    for (int i = 1; i < rows; i++)
    {
        int* data_preRow = _lableImg.ptr<int>(i-1) ;
        int* data_curRow = _lableImg.ptr<int>(i) ;
        for (int j = 1; j < cols; j++)
        {
            if (data_curRow[j] == 1)
            {
                std::vector<int> neighborLabels ;
                neighborLabels.reserve(2) ;
                int leftPixel = data_curRow[j-1] ;
                int upPixel = data_preRow[j] ;
                if ( leftPixel > 1)
                {
                    neighborLabels.push_back(leftPixel) ;
                }
                if (upPixel > 1)
                {
                    neighborLabels.push_back(upPixel) ;
                }

                if (neighborLabels.empty())
                {
                    labelSet.push_back(++label) ;  // assign to a new label
                    data_curRow[j] = label ;
                    labelSet[label] = label ;
                }
                else
                {
                    std::sort(neighborLabels.begin(), neighborLabels.end()) ;
                    int smallestLabel = neighborLabels[0] ;
                    data_curRow[j] = smallestLabel ;

                    // save equivalence
                    for (size_t k = 1; k < neighborLabels.size(); k++)
                    {
                        int tempLabel = neighborLabels[k] ;
                        int& oldSmallestLabel = labelSet[tempLabel] ;
                        if (oldSmallestLabel > smallestLabel)
                        {
                            labelSet[oldSmallestLabel] = smallestLabel ;
                            oldSmallestLabel = smallestLabel ;
                        }
                        else if (oldSmallestLabel < smallestLabel)
                        {
                            labelSet[smallestLabel] = oldSmallestLabel ;
                        }
                    }
                }
            }
        }
    }
    // update equivalent labels
    // assigned with the smallest label in each equivalent label set
    for (size_t i = 2; i < labelSet.size(); i++)
    {
        int curLabel = labelSet[i] ;
        int preLabel = labelSet[curLabel] ;
        while (preLabel != curLabel)
        {
            curLabel = preLabel ;
            preLabel = labelSet[preLabel] ;
        }
        labelSet[i] = curLabel ;
    }


    // 2. second pass
    for (int i = 0; i < rows; i++)
    {
        int* data = _lableImg.ptr<int>(i) ;
        for (int j = 0; j < cols; j++)
        {
            int& pixelLabel = data[j] ;
            pixelLabel = labelSet[pixelLabel] ;
        }
    }
}

/** 找出連通物件(方法三) **/
void ProcessThread::FindBlobs(const cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs)
{
    //cout << "ProcessThread::FindBlobs";
    blobs.clear();

    // Fill the label_image with the blobs
    // 0  - background
    // 1  - unlabelled foreground
    // 2+ - labelled foreground

    cv::Mat label_image;
    binary.copyTo(label_image);
    cv::threshold(binary, label_image, 0, 1, cv::THRESH_BINARY);
    //binary.copyTo(label_image);
    int label_count = 2; // starts at 2 because 0,1 are used already

    //利用floodfill函式(opencv)找出連通物件
    for(int y=0; y < label_image.rows; y++) {
        uchar *row = label_image.ptr<uchar>(y);
        for(int x=0; x < label_image.cols; x++) {
            if(row[x] != 1) {
                continue;
            }
            cv::Rect rect;
            cv::floodFill(label_image, cv::Point(x,y), label_count, &rect, 0, 0, 8);

            std::vector <cv::Point2i> blob;

            for(int i=rect.y; i < (rect.y+rect.height); i++) {
                uchar *row2 = label_image.ptr<uchar>(i);
                for(int j=rect.x; j < (rect.x+rect.width); j++) {
                    if(row2[j] != label_count) {
                        continue;
                    }

                    blob.push_back(cv::Point2i(j,i));
                }
            }

            if (blob.size() > BLOB_THRESHOLD)
                blobs.push_back(blob);

            label_count++;
        }
    }
}

/** 回傳兩Rect是否相交 **/
int ProcessThread::IntersectionCheck (Rect a, Rect b)
{
    int a_x1 = a.x;
    int a_x2 = a_x1 + a.width - 1;
    int a_y1 = a.y;
    int a_y2 = a_y1 + a.height - 1;

    int b_x1 = b.x;
    int b_x2 = b_x1 + b.width - 1;
    int b_y1 = b.y;
    int b_y2 = b_y1 + b.height - 1;

    if ( (a_x1 < b_x1 || a_x1 < b_x2 ) && (a_x2 > b_x1 || a_x2 > b_x2) && (a_y1 < b_y1 || a_y1 < b_y2 ) && (a_y2 > b_y1 || a_y2 > b_y2) )
        return 1;
    else
        return 0;

}

/** 記錄物件為事件 **/
int ProcessThread::RecordObject (trackingObject obj, string rName,vector <Rect> exitH, vector <Rect> exitV)
{
    fstream reasonsFile;
    reasonsFile.open("reasons.txt", ios::out|ios::app);
    if(!reasonsFile){    //如果開啟檔案失敗，fp為0；成功，fp為非0
        cout<<"Fail to open file: "<< "reasons" <<endl;
        return -1;
    }

    int ck = checkEvent(obj);

    int tSize = obj.trajectoryTime.size();
    if (ck == 1) {
        reasonsFile << obj.id << " - " << "時間太短" << endl;
        reasonsFile.close();
        return -1;
    }
    float avgArea = 1.0*obj.sumArea / tSize;
    float avgHeight = 1.0*obj.sumHeight / tSize;
    float avgWidth = 1.0*obj.sumWidth / tSize;
    if (ck == 2) {
        reasonsFile << obj.id << " - " << "高或寬太小" << " - " << avgHeight << " - " << avgWidth<< endl;
        reasonsFile.close();
        return -1;
    }
    if (ck == 3) {
        reasonsFile << obj.id << " - " << "移動距離太短都沒動" << endl;
        reasonsFile.close();
        return -1;
    }


    fstream recordFile;
    recordFile.open(rName.c_str(), ios::out|ios::app);
    if(!recordFile){    //如果開啟檔案失敗，fp為0；成功，fp為非0
        cout<<"Fail to open file: "<< rName <<endl;
        return -1;
    }

    recordFile << obj.id << " " << tSize << " " << avgArea << " " << avgHeight << " " << avgWidth <<endl;

    int humanFlag = checkHuman(obj);
    int eventType = 0;

    if (humanFlag == 0) {   //vehicle
        int startPoint = checkPosition(obj.trajectoryPoint[0],exitV);
        int endingPoint = checkPosition(obj.trajectoryPoint[tSize-1],exitV);
        if (startPoint == 1 && endingPoint == 0) {    //從外面進來 裡面消失 =>停車
            recordFile << PARKING << endl;
            eventType = PARKING;
        }
        if (startPoint == 1 && endingPoint == 1) {   //從外面進來 又跑出去 =>經過
            recordFile << DRIVE_THROUGH << endl;
            eventType = DRIVE_THROUGH;
        }
        if (startPoint == 0 && endingPoint == 1) {   //從裡面出現 跑出去 =>開走
            recordFile << DRIVE_AWAY << endl;
            eventType = DRIVE_AWAY;
        }
        if (startPoint == 0 && endingPoint == 0) {   //從裡面出現 裡面消失 =>換停車位
            recordFile << CHANGE_PARKINGLOT << endl;
            eventType = CHANGE_PARKINGLOT;
        }
    }
    else {  //human
        int startPoint = checkPosition(obj.trajectoryPoint[0],exitH);
        int endingPoint = checkPosition(obj.trajectoryPoint[tSize-1],exitH);
        if (startPoint == 1 && endingPoint == 0) {    //從外面進來 裡面消失 =>上車
            recordFile << GET_INTO_CAR << endl;
            eventType = GET_INTO_CAR;
        }
        if (startPoint == 1 && endingPoint == 1) {   //從外面進來 又跑出去 =>經過
            recordFile << WALK_THROUGH << endl;
            eventType = WALK_THROUGH;
        }
        if (startPoint == 0 && endingPoint == 1) {   //從裡面出現 跑出去 =>下車走掉
            recordFile << GET_OFF_CAR << endl;
            eventType = GET_OFF_CAR;
        }
        if (startPoint == 0 && endingPoint == 0) {    //從裡面出現 裡面消失 =>下車又上車
            recordFile << GO_BACK_TO_CAR << endl;
            eventType = GO_BACK_TO_CAR;
        }
    }

    for (int i=0; i<tSize; ++i) {
        recordFile << obj.trajectoryTime[i] << " " << obj.trajectoryPoint[i].x << " " << obj.trajectoryPoint[i].y << " " << obj.trajectoryRect[i].x << " " << obj.trajectoryRect[i].y << " " << obj.trajectoryRect[i].width << " " << obj.trajectoryRect[i].height << endl;
        //        if (i != 0 && i%10 == 0) {
        //            int spd = pow(obj.trajectoryPoint[i].x - obj.trajectoryPoint[i-10].x, 2) + pow(obj.trajectoryPoint[i].y - obj.trajectoryPoint[i-10].y, 2);
        //            if (spd > SPEED_THRESHOLD)
        //                humanFlag = 0;
        //            cout << obj.id << "  " << spd << endl;
        //        }
    }

    recordFile.close();
    return eventType;
}

/** 回傳是否能被記錄為事件 **/
int ProcessThread::checkEvent(trackingObject obj)
{
    int tSize = obj.trajectoryTime.size();
    if (tSize*NUM_FRAME_PROCESS < EVENT_TIME) {
        return 1;
    }
    float avgArea = 1.0*obj.sumArea / tSize;
    float avgHeight = 1.0*obj.sumHeight / tSize;
    float avgWidth = 1.0*obj.sumWidth / tSize;
    //長或寬太小(雜訊)
    if (avgHeight < NOISE_HEIGHT_WIDTH_THRESHOLD || avgWidth < NOISE_HEIGHT_WIDTH_THRESHOLD) {
        return 2;
    }
    //從頭到尾都未移動 (樹葉、樹影)
    if ( (pow(obj.trajectoryPoint[tSize-1].x - obj.trajectoryPoint[0].x, 2) + pow(obj.trajectoryPoint[tSize-1].y - obj.trajectoryPoint[0].y, 2)) < NO_MOVE_THRESHOLD) {
        int moveFlag = 0;
        for (int i=1; i<tSize; ++i) {
            if ( (pow(obj.trajectoryPoint[i].x - obj.trajectoryPoint[0].x, 2) + pow(obj.trajectoryPoint[i].y - obj.trajectoryPoint[0].y, 2)) > NO_MOVE_THRESHOLD) {
                moveFlag = 1;
                break;
            }
        }
        if (moveFlag == 0) {
            return 3;
        }
    }

    return 0;
}

/** 回傳物件類型 (人/車) **/
int ProcessThread::checkHuman(trackingObject obj)
{
    int tSize = obj.trajectoryTime.size();
    float avgArea = 1.0*obj.sumArea / tSize;
    float avgHeight = 1.0*obj.sumHeight / tSize;
    float avgWidth = 1.0*obj.sumWidth / tSize;

    float hwRatio = 1.0*avgHeight/avgWidth;
    int objFlag;
    if (avgArea > SIZE_THRESHOLD) objFlag = 0;  //car
    else {
        if (hwRatio > HEIGHT_WIDTH_RATIO) objFlag = 1;     //小於SIZE_TH;長寬比大於定義 = 人
        else objFlag = -1;  //既小於SIZE_TH高寬比又不符合>>雜訊
    }
    return objFlag;
}

/** 回傳該點是否在其中一個rect裡面 **/
int ProcessThread::checkPosition(Point pos, vector <Rect> exit)
{
    for (int i=0; i<exit.size(); ++i) {
        if (pos.x > exit[i].x && pos.y > exit[i].y && pos.x < (exit[i].x+exit[i].width) && pos.y < (exit[i].y+exit[i].height))
            return 1;
    }
    //if (pos.x < EXIT_DISTANCE || (frameWidth-pos.x) < EXIT_DISTANCE || pos.y < EXIT_DISTANCE || (frameHeight-pos.y) < EXIT_DISTANCE)
    //  return 1;

    return 0;
}

/** 回傳一點與一個Rect的垂直距離 **/
int ProcessThread::pointRectDistance(Point a, Rect b)
{
    int ax = a.x;
    int ay = a.y;
    int bx1 = b.x;
    int bx2 = b.x+b.width;
    int by1 = b.y;
    int by2 = b.y+b.height;
    if (ax < bx1) { //左邊
        if (ay<by1) {  //上面
            return (pow((ax-bx1),2)+pow((ay-by1),2));
        }
        else if (ay > by2) { //下面
            return (pow((ax-bx1),2)+pow((ay-by2),2));
        }
        else {  //中間(垂直方向)
            return pow((ax-bx1),2);
        }
    }
    else if (ax > bx2) { //右邊
        if (ay<by1) {  //上面
            return (pow((ax-bx2),2)+pow((ay-by1),2));
        }
        else if (ay > by2) { //下面
            return (pow((ax-bx2),2)+pow((ay-by2),2));
        }
        else {  //中間(垂直方向)
            return pow((ax-bx2),2);
        }
    }
    else {  //中間(水平方向)
        if (ay<by1) {  //上面
            return  pow((ay-by1),2);
        }
        else if (ay > by2) { //下面
            return  pow((ay-by2),2);
        }
        else {  //中間(垂直方向)
            return 0;
        }
    }

}

/** 回傳是否其中一個Rect被另一個Rect完全包覆 **/
int ProcessThread::isCovered(Rect a, Rect b)
{
    if (a.x >= b.x && a.y >= b.y && a.width <= b.width && a.height <= b.height)
        return 1;   //a is covered by b
    if (b.x >= a.x && b.y >= a.y && b.width <= a.width && b.height <= a.height)
        return 2;   //b is covered by a

    return 0;   //not covered
}

/** 在畫面上畫出指定的Rect **/
void ProcessThread::drawRect(Mat& paint, Rect a, int g, int b, int r, int thick)
{
    Point tl = Point(a.x,a.y);
    Point tr = Point(a.x+a.width,a.y);
    Point bl = Point(a.x,a.y+a.height);
    Point br = Point(tr.x,bl.y);
    line(paint,tl,tr,Scalar(b,g,r),thick);
    line(paint,tl,bl,Scalar(b,g,r),thick);
    line(paint,br,tr,Scalar(b,g,r),thick);
    line(paint,br,bl,Scalar(b,g,r),thick);
    return;
}

/** 印出Mat的內容 **/
void ProcessThread::printMat (Mat p)
{
    for(int i = 0; i < p.rows; i++) {
        for(int j = 0; j < p.cols; j++) {
            if (p.at<uchar>(i,j,0) > 180) cout << "!!!!!!" ;
            //cout << (int)p.at<uchar>(i,j,0) << " ";
        }
    }
}

/** 統計直方圖(HSV) **/
void ProcessThread::calcWeightedHistogram(Mat frame, const Rect roi, Mat &destHist, const Mat *mask)    //frame=hsv
{
    Mat image_roi = frame(roi);

    int img_width = image_roi.cols;
    int img_height = image_roi.rows;
    Point2f roi_center(0.5*img_width, 0.5*img_height);
    int hx = img_width*0.5 , hy = img_height*0.5;

    destHist.create(3, histSize, CV_32F); //3 dimention histogram
    destHist.flags = (destHist.flags & ~CV_MAT_TYPE_MASK)|CV_32F;
    destHist = Scalar(0); //fill zero

    Vec3b* pix; //pixel reader
    uchar *maskPtr; //mask pixel reader

    double normal_c = 0.0; //the couter to do normalize

    if (mask != NULL){
        Mat mask_roi = (*mask)(roi);
        for (int y = 0; y < img_height; ++y) {
            pix  = image_roi.ptr<Vec3b>(y);
            maskPtr = mask_roi.ptr<uchar>(y);
            for (int x = 0; x < img_width; ++x) {
                if (maskPtr[x] > 0){
                    uchar h_i = cvFloor(pix[x][0]*histSize[0]/256.0),
                            s_i = cvFloor(pix[x][1]*histSize[1]/256.0),
                            v_i = cvFloor(pix[x][2]*histSize[2]/256.0);
                    float weighted_value = kernelFunc(roi_center, Point2f(x,y), hx, hy); //kernel function
                    destHist.at<float>(h_i, s_i, v_i) += weighted_value;
                    normal_c += weighted_value;
                }
            }
        }
    }
    else{
        for (int y = 0; y < img_height; ++y) {
            pix  = image_roi.ptr<Vec3b>(y);
            for (int x = 0; x < img_width; ++x) {
                uchar h_i = cvFloor(pix[x][0]*histSize[0]/256.0),
                        s_i = cvFloor(pix[x][1]*histSize[1]/256.0),
                        v_i = cvFloor(pix[x][2]*histSize[2]/256.0);
                float weighted_value = kernelFunc(roi_center, Point2f(x,y), hx, hy); //kernel function
                destHist.at<float>(h_i, s_i, v_i) += weighted_value;
                normal_c += weighted_value;
            }
        }
    }

    //Normalize
    if (normal_c != 0){
        for (int i = 0; i < h_bins; ++i)
            for (int j = 0; j < s_bins; ++j)
                for (int k = 0; k < v_bins; ++k){
                    destHist.at<float>(i, j, k) /= normal_c;
                }
    }
}

/** 統計直方圖使用的Kernel **/
float ProcessThread::kernelFunc(Point2f& roi_center, Point2f xy, int hx, int hy)
{
    float distx = (xy.x - roi_center.x)/hx;
    float disty = (xy.y - roi_center.y)/hy;
    float dist_square = distx*distx + disty*disty;

    if (dist_square >= 1)
        return 0;

    return (1.0-dist_square)*(1.0-dist_square)*2;
}

/** 將直方圖normalize到0到255 **/
void ProcessThread::hist0To255(Mat &destHist)
{
    //Normalize the value from 0 to 255
    float max_value = 0.0;
    for (int i = 0; i < h_bins; ++i)
        for (int j = 0; j < s_bins; ++j)
            for (int k = 0; k < v_bins; ++k){
                if (destHist.at<float>(i, j, k) > max_value)
                    max_value = destHist.at<float>(i, j, k);
            }
    max_value /= 256;
    for (int i = 0; i < h_bins; ++i)
        for (int j = 0; j < s_bins; ++j)
            for (int k = 0; k < v_bins; ++k)
                destHist.at<float>(i, j, k) = (destHist.at<float>(i, j, k)/max_value);
}

/** 判斷是否符合可疑人物行為 **/
int ProcessThread::checkThief(trackingObject obj,vector <Rect> cars)
{
    if (obj.trajectoryTime.size() < 500) return -1;
    if (obj.trajectoryTime.size()%10 != 0) return -1;
    if (checkHuman(obj) != 1) return -1;
    if (obj.thief == 1) return -1;

    int aroundTime[cars.size()];
    for (int m=0;m<cars.size();++m) aroundTime[m]=0;

    for (int l=0; l<obj.trajectoryTime.size(); ++l) {
        for (int m=0;m<cars.size();++m) {
            if (pointRectDistance(obj.trajectoryPoint[l], cars[m]) < THIEF_DISTANCE)
                ++aroundTime[m];
        }
    }
    for (int m=0;m<cars.size();++m) {
        if (aroundTime[m] > 500) {  //在一車旁繞超過一分鐘
            return m;
        }
    }
    return -1;
}


// ========== 指定偵測區域 物件 ============
DetectAssignArea::DetectAssignArea()
{
    this->diffPixels = 0;
    this->diffThreshold = 0;
    this->alert = false;
    this->logAlertRecord = false;
}
// Return 前景和背景二值化的差異Pixel數目
int DetectAssignArea::getDiffPixels()
{
    return this->diffPixels;
}
// Return 差異Pixel數目門檻值(若超過則警報)
int DetectAssignArea::getDiffThreshold()
{
    return this->diffThreshold;
}
// Return 觸發警報與否
bool DetectAssignArea::getAlert()
{
    return this->alert;
}
// Return Log是否有紀錄過該警報事件
bool DetectAssignArea::getLogAlertRecord()
{
    return this->logAlertRecord;
}
// 設定 前景和背景二值化的差異Pixel數目
void DetectAssignArea::setDiffPixels(const int &pixels)
{
    this->diffPixels = pixels;
}
// 設定 差異Pixel數目門檻值(若超過則警報)
void DetectAssignArea::setDiffThreshold(const int &threshold)
{
    this->diffThreshold = threshold;
}
// 設定 警告觸發
void DetectAssignArea::setAlert(const bool &alert)
{
    this->alert = alert;
}
// 設定 Log是否有紀錄過該警報事件
void DetectAssignArea::setLogAlertRecord(const bool &alertRecord)
{
    this->logAlertRecord = alertRecord;
}
