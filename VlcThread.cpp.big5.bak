#include "VlcThread.h"
#include <QtDebug>
#include <QMutex>
#include <QWaitCondition>
#include "outputLog.h"
// ===== 全域變數 =====
Mat vlcShowFrame,vlcPushFrame;//顯示UI用
QMutex isNewDataMutex;
bool global_doStop;//Control thread flag
bool isNewData;
ConcurrentQueue<Mat> *global_queue;//Frame Buffer
int push_counter,player_counter;//counter

outputLog VlcThreadLog("VLCThread.log");

VlcThread::VlcThread(MainWindow* main_window,ConcurrentQueue<Mat> *queue,const bool openVideo)
{
    VlcThreadLog.printLog(logAddMem("VlcThread::VlcThread() enter"));
    this->frame_queue = queue;//Frame Buffer
    this->openVideo = openVideo;
    global_queue = queue;//Frame Buffer
    global_doStop = false;//Set Action Flag
    isNewData = false;
    push_counter = 0;
    player_counter = 0;
    //更新主視窗影像畫面
    QObject::connect(this,SIGNAL(processedImage(const QImage)),
                     main_window,SLOT(updatePlayerUI(const QImage)));
    //delete the thread when quit
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    VlcThreadLog.printLog(logAddMem("VlcThread::VlcThread() exit"));
}

VlcThread::~VlcThread()
{
    VlcThreadLog.printLog(logAddMem("VlcThread::~VlcThread()"));
}

//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ VLC API ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
//Purpose : Get the Frame Data and convert it to the Mat
void *lock(void *data, void **p_pixels)
{
    struct ctx *ctx = (struct ctx*)data;
    WaitForSingleObject( ctx->mutex, INFINITE );
    *p_pixels = ctx->pixels;
    return NULL;
}

void display(void *data, void *id)
{
    (void) data;
    assert( id == NULL );
}

//◎主要拿到Frame Data 的 Function
void unlock(void *data, void *id, void * const *p_pixels)
{
    struct ctx *ctx = (struct ctx*)data;
    Mat img = *ctx->image;
    //=== !!重要!! 複製Frame,一份給Push用,一份給顯示用 ===
    img.copyTo(vlcShowFrame);
    img.copyTo(vlcPushFrame);
    //告訴Push Thread：有新的Frame
    isNewDataMutex.lock();
    isNewData = true;
    isNewDataMutex.unlock();
    ReleaseMutex( ctx->mutex );
    //    qDebug() << "player counter:" << player_counter;
}
//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ VLC API (End)↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑

//********** VLC Open Streaming and Initialization ********
void VlcThread::VLC_Streaming_Init(const char* open_streaming_addr,int frame_width, int frame_height)
{
    VlcThreadLog.printLog(logAddMem("VlcThread::VLC_Streaming_Init() enter"));
    const char *const vlc_args[] = {
        "-I",
        "dummy",
        "--ignore-config",
//        "--extraintf=logger",
        "--verbose=2",
    };
    int vlc_argc = sizeof(vlc_args)/sizeof(vlc_args[0]);
    //用於初始化一個libvlc的實例，argc表示參數的個數，argv表示參數，返回創建的實例若當發生錯誤時返回NULL
    vlcInstance = libvlc_new( vlc_argc, vlc_args );
    if( vlcInstance == NULL ){
        std::cout<<"Create Media Stream Error"<<std::endl;
        VlcThreadLog.printLog(logAddMem("VlcThread::VLC_Streaming_Init() exit1(Create Error)"));
        return ;
    }
    //使用一個給定的媒體資源路徑來建立一個libvlc_media對象，
    //參數psz_mrl為要讀取的MRL(Media Resource Location)，
    //此函數返回新建的對像或NULL.

    if(!openVideo)//Media 1 : 串流網址用
        media = libvlc_media_new_location( vlcInstance, open_streaming_addr);
    else//Media 1 : 串流網址用
        media = libvlc_media_new_path(vlcInstance, open_streaming_addr);

    if( media == NULL ){
        std::cout<<"Media Stream is Null"<<std::endl;
        VlcThreadLog.printLog(logAddMem("VlcThread::VLC_Streaming_Init() exit2(Stream is NULL)"));
        return ;
    }
    //LIBVLC_API libvlc_media_player_t* libvlc_media_player_new_from_media( libvlc_media_t *p_md )
    //根據給定的媒體對象創建一個播放器對象
    mp = libvlc_media_player_new_from_media( media );

    // LIBVLC_API void libvlc_media_release( libvlc_media_t *p_md )
    //減少一個libvlc_media_t的引用計數,如果減少到0時,此函數將釋放此對象(銷毀).
    //它將發送一個libvlc_MediaFreed事件到所有的監聽者那裡。
    //如果一個libvlc_media_t被釋放了，它就再也不能使用了。
    libvlc_media_release(media);

    //初始化含OpenCV Mat的結構參數
    struct ctx* context = (struct ctx*)malloc( sizeof(*context) );
    context->mutex      = CreateMutex( NULL, false, NULL );
    context->image      = new Mat( frame_height, frame_width, CV_8UC3 );
    context->pixels     = (unsigned char *)context->image->data;

    // show blank image
    cv::Mat img = *context->image;

    //設定Callback Function複製影像資訊，可藉由libvlc_video_set_format和libvlc_video_set_format_callbacks設定解碼資訊
    libvlc_video_set_callbacks( mp, lock, unlock, display, context );

    //設定影像解碼資訊
    const char *chroma = "RV24";
    unsigned width     = frame_width;
    unsigned height    = frame_height;
    unsigned pitch     = frame_width*24/8;
    libvlc_video_set_format( mp, chroma, width, height, pitch);
    qDebug() << "===================VLC Open Streaming and Initialization Done.=======================";
    VlcThreadLog.printLog(logAddMem("VlcThread::VLC_Streaming_Init() exit3(Succeed)"));
}

//Stop the Streaming and Release the VLC_Media_Player
void VlcThread::stop()
{
    VlcThreadLog.printLog(logAddMem("VlcThread::stop() enter"));
    QMutexLocker locker(&doStopMutex);
    global_doStop = true;
    VlcThreadLog.printLog(logAddMem("VlcThread::stop() exit"));
}


// ====== Thread 啟動 =======
void VlcThread::run()
{
    qDebug() << "VLC Thread Start to Play the Streaming";
    VlcThreadLog.printLog(logAddMem("VlcThread::run() enter"));
    //開始播放串流影片
    libvlc_media_player_play(mp);

    while(1)
    {
        VlcThreadLog.printLog(logAddMem("VlcThread::run() enter1"));
        ////////////////////////////////
        // Stop thread if doStop=TRUE //
        ////////////////////////////////
        doStopMutex.lock();
        if(global_doStop)
        {
            doStopMutex.unlock();
            break;
        }
        doStopMutex.unlock();
        /////////////////////////////////
        VlcThreadLog.printLog(logAddMem("VlcThread::run() enter2"));
        isNewDataMutex.lock();
        if(isNewData){//有新的串流資料
            if(!vlcPushFrame.empty() && vlcPushFrame.data){//Push Frame
                VlcThreadLog.printLog(logAddMem("VlcThread::run() enter3"));
                global_queue->push_and_pop(vlcPushFrame,100);
                push_counter++;
                if(push_counter==numeric_limits<int>::max() - 500)//避免overflow
                    push_counter = 0;
            }else
                VlcThreadLog.printLog(logAddMem("VlcThread::run() enter3-1(Error: error frame)"));
            VlcThreadLog.printLog(logAddMem("VlcThread::run() enter4"));
            if(!vlcShowFrame.empty() && vlcShowFrame.data)//顯示畫面在處理端
                emit processedImage(Mat2QImage(vlcShowFrame));
            isNewData = false;
            VlcThreadLog.printLog(logAddMem("VlcThread::run() enter5"));
        }
        isNewDataMutex.unlock();
        this->msleep(33);//勿使用waitKey(),會使CPU使用率飆高
        //        qDebug() << "global_doStop:" << global_doStop;
    }
    VlcThreadLog.printLog(logAddMem("VlcThread::run() enter6"));
    //Release the media_player's resource
    libvlc_media_player_stop( mp );
    libvlc_media_player_release( mp );
    libvlc_release( vlcInstance );
    VlcThreadLog.printLog(logAddMem("VlcThread::run() exit"));
}
//影像格式轉換,Mat 轉 QImage
QImage VlcThread::Mat2QImage(Mat const& src)
{
    VlcThreadLog.printLog(logAddMem("VlcThread::Mat2QImage() enter"));
    Mat temp; // make the same cv::Mat
    cvtColor(src, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage dest((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    QImage dest2(dest);
    dest2.detach(); // enforce deep copy
    VlcThreadLog.printLog(logAddMem("VlcThread::Mat2QImage() exit"));
    return dest2;
}

string VlcThread::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}


