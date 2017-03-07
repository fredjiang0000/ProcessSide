#include "CaptureThread.h"
#include "ImgProcess.h"
#include <QtDebug>
#include <ctime>

//Constructor建構子
//目的:抓取影像串流並且把監視器的影像串流Push至Buffer
CaptureThread::CaptureThread(MainWindow* main_window,ConcurrentQueue<Mat> *queue,QString address)
{
    myLog = new outputLog("CaptureThread.log");
    myLog->printLog(logAddMem("CaptureThread::CaptureThread() enter"));
    log_filename = "";
    frame_queue = queue;
    cap_address = address;
    // Initialize variables(s)
    doStop=false;
    sampleNumber=0;
    fpsSum=0;
    averageFPS=0;
    fps.clear();
    queue_counter = 0;
    empty_counter = 0;
    ProcessingThread = NULL;
    //更新主視窗影像畫面
    QObject::connect(this,SIGNAL(processedImage(const QImage)),
                     main_window,SLOT(updatePlayerUI(const QImage)));
    //update the mainwindow's Log
    QObject::connect(this,SIGNAL(PrintMyLog(QString,const int)),
                     main_window,SLOT(UpdateMyLog(QString,const int)));
    //delete the thread when quit
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    myLog->printLog(logAddMem("CaptureThread::CaptureThread() exit"));
}

//Destructor 解構子
CaptureThread::~CaptureThread()
{
    myLog->printLog(logAddMem("CaptureThread::~CaptureThread()"));
    delete this->myLog;
    this->myLog = NULL;
}


//開啟 串流網址
bool CaptureThread::connectToCamera(const string stream_addr)
{
    myLog->printLog(logAddMem("CaptureThread::connectToCamera() enter1"));
    // Open camera
    bool camOpenResult = cap.open(stream_addr);
    //成功Open Camera時,避免讀不到frame_width和frame_height,檢查讀不讀的到frame
    if(camOpenResult)
    {
        while(!cap.read(src_frame))//讀不到則重新開起串流
        {
            cap.release();
            cap.open(stream_addr);
            //wait
            continue;
        }
        myLog->printLog(logAddMem("CaptureThread::connectToCamera() enter2(succeed)"));
    }
    // Set resolution
    this->frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    this->frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    cout << "CaptureThread frame_width : " << frame_width<< endl;
    cout << "CaptureThread frame_height : " << frame_height<< endl;

    string log_msg= "CaptureThread::connectToCamera() enter3(succeed)(frame_width:";
    log_msg += to_string(frame_width); log_msg+=" frame_height:"; log_msg+= to_string(frame_height); log_msg+=")";
    myLog->printLog(logAddMem(log_msg));
    // Return result
    return camOpenResult;
}
//關閉 串流網址
bool CaptureThread::disconnectCamera()
{
    // Camera is connected
    if(cap.isOpened())
    {
        // Disconnect camera
        cap.release();
        return true;
    }
    // Camera is NOT connected
    else
        return false;
}

//停止Thread
void CaptureThread::stop()
{
    QMutexLocker locker(&doStopMutex);
    doStop=true;
    cout << "Capture Thread doStop = true"<< endl;
}

bool CaptureThread::isCameraConnected()
{
    return cap.isOpened();
}

//開始執行 Thread->Start
void CaptureThread::run()
{
    //*******************Output Log************************
    string log_msg = "開始抓取監視器影像串流";
    ProcessingThread->Log_for_display.push_back(log_msg);
    Log_Msg = "開始抓取監視器影像串流";
    emit PrintMyLog(Log_Msg,0);
//    Output_Log("開始抓取監視器影像串流");
    //*******************Output Log(End)*******************
    cout << "Capture Thread Start"<< endl;
    while(1)
    {
        //        cout << "CaptureThread 1";
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
        //=== 開始 Push Frame 到 Queue Buffer 中 ===
            cout << "CaptureThread 2"<< endl;
            if(!cap.read(src_frame))
                continue;
            if(src_frame.total()!=frame_width*frame_height)
            {
                cout << "Frame's resulution doesn't match, Drop the frame!"<< endl;
                continue;
            }

            cout << "CaptureThread 3"<< endl;

            //!重要,複製一份原始影像給顯示的Frame, 一份給Push的Frame
            //原因:其他地方更改到原始影像的值,呼叫更新UI畫面會當機
            src_frame.copyTo(show_frame);
            src_frame.copyTo(push_frame);
            //更新UI畫面 Update MainWindow's UI (原解析度)
            emit processedImage(Mat2QImage(show_frame));
            cout << "CaptureThread 4"<< endl;

            //*********計算[Capture]Push一張frame所花費的時間,用途:更新FPS****
            // Save capture time
            captureTime=cap_t.elapsed();
            // Start timer (used to calculate capture rate)
            cap_t.start();
            //************************************************************

            //Push a frame to the queue (上限:100)
            frame_queue->push_and_pop(push_frame,100);
                queue_counter++;

            cout << "CaptureThread 5"<< endl;
            //overflow protection
            if(queue_counter==numeric_limits<int>::max() - 500)
                queue_counter = 0;

            //**********************更新並印出 Capture FPS*************************
            // Update statistics
            updateFPS(captureTime);
//            time_t now_t = time(0);// get time now
//            struct tm * now = localtime( & now_t );
//            qDebug() << (now->tm_year + 1900) << '/'
//                 << (now->tm_mon + 1) << '/'
//                 <<  now->tm_mday
//                  <<  now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec
//                   << "Capture FPS:"<< averageFPS
//                   <<" push " << queue_counter << "th frames into queue";
            //********************************************************************

        cout << "CaptureThread 6"<< endl;
    }//end of while loop
    cout << "Capture Thread quits!(Break the loop)"<< endl;
//    Output_Log("Capture Thread quits!(Break the loop)");

    //Send NULL Qimg to set MainWindow's Label to Black BackGround
    QImage qimg = QImage();
    emit processedImage(qimg);
    this->quit();
}

//暫停 Thread
void CaptureThread::pause()
{
    QMutexLocker locker(&doPauseMutex);
    isPause = true;
    cout << "CaptureThread Thread isPause = true"<< endl;
}
//繼續 Thread
void CaptureThread::unpause()
{
    QMutexLocker locker(&doPauseMutex);
    isPause = false;
    cout << "CaptureThread Thread isPause = false"<< endl;
}
//更新 FPS
void CaptureThread::updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed>0)
    {
        fps.enqueue((int)1000/timeElapsed);
        // Increment sample number
        sampleNumber++;
    }
    // Maximum size of queue is DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH
    if(fps.size()>CAPTURE_FPS_STAT_QUEUE_LENGTH)
        fps.dequeue();
    // Update FPS value every DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH samples
    if((fps.size()==CAPTURE_FPS_STAT_QUEUE_LENGTH)&&(sampleNumber==CAPTURE_FPS_STAT_QUEUE_LENGTH))
    {
        // Empty queue and store sum
        while(!fps.empty())
            fpsSum+=fps.dequeue();
        // Calculate average FPS
        averageFPS=fpsSum/CAPTURE_FPS_STAT_QUEUE_LENGTH;
        // Reset sum
        fpsSum=0;
        // Reset sample number
        sampleNumber=0;
    }
}
//影像格式轉換,Mat 轉 QImage
QImage CaptureThread::Mat2QImage(const Mat &src)
{
    Mat temp; // make the same cv::Mat
    cvtColor(src, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage dest((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    QImage dest2(dest);
    dest2.detach(); // enforce deep copy
    return dest2;
}





////Output Log訊息至Log.txt
//void CaptureThread::Output_Log(const string &messages)
//{
//    ofstream mylog;
//    //依照日期打開對應日期的txt檔
//    log_filename = "Log "; log_filename += get_current_date(); log_filename += ".txt";
//    mylog.open(log_filename,ios::out | ios::app);
//    if(!mylog)
//    {
//        cout << "[Error] Fail to open MyLog"<< endl;
//        exit(1);
//    }
//    else
//    {
//        get_current_time(mylog);
//        mylog << "  " << ProcessingThread->cam_location <<"  [Capture] " << messages << endl;
//        mylog.close();
//    }
//}

string CaptureThread::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}








