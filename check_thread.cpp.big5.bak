#include "check_thread.h"
#include "warningmessage.h"

int e_num=0;
int p_num=0;

extern outputLog checkThreadLog("check_thread.log");
/********************
 * [Func] : 建構子
 * srcFrame : 欲判斷圖片串列
*************************/
check_thread::check_thread(){
    checkThreadLog.printLog(logAddMem("check_thread::check_thread() enter"));
    this->doStop = false;
    QObject::connect( this, SIGNAL(finished()),this, SLOT(deleteLater()));
    checkThreadLog.printLog(logAddMem("check_thread::check_thread() exit"));
}
/*************************
 * [Func] : 解構子
**************************/
check_thread::~check_thread(){
    checkThreadLog.printLog(logAddMem("check_thread::~check_thread()"));
}
/*****************************************
 *〔Func〕手機辨識thread
*****************************************/
void check_thread::run(){
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::run() enter"));
    int counter=0;
    while(1){
        doStopMutex.lock();
        if(doStop)
        {
            doStop=false;
            doStopMutex.unlock();
            break;
        }
        doStopMutex.unlock();
        checkThreadLog.printLog(logAddMem("check_thread thread check_thread::run() enter1"));
        if(srcFrame.size()> 0){ // 串列大小不為 0 ，判斷第一張圖
            checkThreadLog.printLog(logAddMem("check_thread thread check_thread::run() enter2"));
            Mat img = srcFrame[0];
            temp_rect = srcRect[0];
            result = cellDetect(img);
            /**********傳送 警告訊息 *************/
            if(result == true){
                Set_result_ROI(true);
                imwrite(this->saveFilePath(counter),img);
                warningmessage warnMsg;
                warnMsg.setWarning(true);
            }
            /************************************/
            mutex_erase();
            counter++;
            checkThreadLog.printLog(logAddMem("check_thread thread check_thread::run() enter3"));
        }
        this->msleep(33);
    }
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::run() exit"));
}

/**************************
 * [Func]: erase 刪除圖片串列第一張
**************************/
void check_thread::mutex_erase(){
  checkThreadLog.printLog(logAddMem("check_thread thread check_thread::mutex_erase() enter"));
  mutex.lock();
  srcFrame.erase(srcFrame.begin());
  srcRect.erase(srcRect.begin());
  mutex.unlock();
  checkThreadLog.printLog(logAddMem("check_thread thread check_thread::mutex_erase() exit"));
}


/**************************
 * [Func]: Push 新增欲判斷圖片
**************************/
void check_thread::mutex_push(Mat frame,Rect rect){
  checkThreadLog.printLog(logAddMem("check_thread thread check_thread::mutex_push() enter"));
  mutex.lock();
  srcFrame.push_back(frame);
  srcRect.push_back(rect);
  mutex.unlock();
  checkThreadLog.printLog(logAddMem("check_thread thread check_thread::mutex_push() exit"));
}
/*************************
 * [Func] : 停止thread
*************************/
void check_thread::stop()
{
    QMutexLocker locker(&doStopMutex);
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::stop() enter"));
    this->doStop = true;
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::stop() exit"));
}
/************************
 * [Func] : Get ROI
************************/
bool check_thread::GetRoi(Rect &roi){
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::GetRoi() enter"));
    roi = temp_rect;
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::GetRoi() exit"));
    return result;
}
/***********************
 * [Func] : 控制紅框訊號
***********************/
void check_thread::Set_result_ROI(bool bottom){
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::Set_result_ROI() enter"));
    ROI_mutex.lock();
    result_ROI = bottom;
    ROI_mutex.unlock();
    checkThreadLog.printLog(logAddMem("check_thread thread check_thread::Set_result_ROI() exit"));
}

string check_thread::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}

string check_thread::saveFilePath(const int count)
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
    string filePath="cell_phone/cell_detection/";
    filePath += to_string(newtime.tm_year + 1900);
    filePath += "-"; filePath += to_string(newtime.tm_mon + 1);
    filePath += "-"; filePath += to_string(newtime.tm_mday);
    filePath += " "; filePath +=to_string(newtime.tm_hour);
    filePath += "-"; filePath +=to_string(newtime.tm_min);
    filePath += "-"; filePath +=to_string(newtime.tm_sec);
    filePath += "_"; filePath +=to_string(count); filePath +=".jpg";
    return filePath;
}
