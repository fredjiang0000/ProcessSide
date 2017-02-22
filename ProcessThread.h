#ifndef ProcessThread_H
#define ProcessThread_H

//Qt
#include <QtCore>
#include <QHostInfo>
#include <QThread>
#include <QTcpSocket>
#include <QAbstractSocket>

//Opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//Local
#include <imgprocess.h>
#include <mainwindow.h>
#include <ConcurrentQueue.h>
#include "TruckRecognize.h"//公司車判斷
#include "cellphone_detection.h"//手機行為判斷
//Win Socket
#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif
//C&C++
#include <unistd.h>
#include <iostream>
using namespace cv;
using namespace std;

class MainWindow;

// ===== 偵測指定區域物體進出 =====
class DetectAssignArea{
public:
    DetectAssignArea();
    int getDiffPixels();
    int getDiffThreshold();
    bool getAlert();
    bool getLogAlertRecord();
    void setDiffPixels(const int& pixels);
    void setDiffThreshold(const int& threshold);
    void setAlert(const bool& alert);
    void setLogAlertRecord(const bool& alertRecord);
protected:
private:
    int diffPixels;
    int diffThreshold;
    bool alert;
    bool logAlertRecord;
};

class ProcessThread : public QThread
{
    Q_OBJECT
public:
    ProcessThread(MainWindow* mainwindow,ConcurrentQueue<Mat>* queue,
                  int frame_width, int frame_height, int FPS, char* cam_location,
                  char* server_ip,const int server_port);
    ~ProcessThread();
    bool isIPAddressAlive(const QString ping_ip, const int port);
    //Initialize
    void initialization(VideoCapture& cap);
    //連線至顯示端
    bool Connect_to_Server(char* server_ip,int server_port);
    //Controll thread function and variables
    void stop();
    //暫存Log的訊息(用來傳送給顯示端)
    vector <string> Log_for_display;
    MainWindow *thread_main_wd;
    //Camera's Info.
    QString rtsp_address,cam_ip;
    int cam_port;
    char* cam_location;
    //For Reconnecting
    char* server_ip;
    int server_port;
    //Set conditions
    vector <Rect> ParkingLot_ROI;
    vector <Rect> ParkingLot_ROI_Not_Update;
    vector <Point> ParkingLot_Display;
    bool Condition_ParkingLot;
    //Video Streaming
    string videoStreamAddress;
    Mat frame;
    String windows_name;//For threads' imshow(Debug used)
    //Video Info.
    int frameWidth,frameHeight,Cap_FPS,Delay;
signals:
    void PrintMyLog(QString msg,const int level);
protected:
    void run();
private:
    //壓縮參數
    vector<uchar> frameBuff;
    vector<int> params;
    //偵測物體進出指定區域的物件
    DetectAssignArea *detectObjectInOutROI;
    //偵測使用手機的Thread
    cellphone_detection *phone_thread;
    //共用的Frame Buffer
    ConcurrentQueue<Mat>* frame_queue;
    //重新連線到顯示端
    void reConnectToDisplay(char *server_ip, int server_port);
    //剛連上顯示端時,送給顯示端監視器的資訊
    bool Send_Camera_Info();
    //檢查接收資料是否Timeout,是的話顯示錯誤訊息並等待重新連線
    bool Recv_and_Check_Timeout(SOCKET connection,char* buf,int buf_len);
    //檢查傳送資料是否Timeout,是的話顯示錯誤訊息並等待重新連線
    bool Send_and_Check_Timeout(SOCKET connection,char* buf,int buf_len);
    //原始和固定後解析度比例
    double width_ratio,height_ratio;
    //是否固定解析度(超過800x600就固定為800x600)
    bool fixed_resolution;
    //【影像處理功能】
    void Motion_Detect();//動態物件追蹤(目前沒用到)
    void ROI_Detect_Objects(const Mat& frame);//指定區域偵測物體
    bool is_ASE_Car(Mat input_frame);//公司車判斷
    //傳送偵測事件的參數
    int Event_Detect_Code;
    double detect_proportion;//靈敏度:判斷指定偵測區域的比例
    int detect_phone_sensitivity;//靈敏度:連續?張Frame都疑似使用手機才做握物和手機判斷
    //取得前景(灰階)
    Mat Get_ForeGround(const Mat& src_frame);
    //只更新指定區域以外的背景
    void Update_BG_Not_ROI();
    //指定區域(n)有物體進出,記錄Log事件
    void recordObjectInOut(const int index);
    void recordObjectDisappear(const int index);
    //畫出有物體佔據的指定區域
    void Draw_ROI_Detect(Mat& frame);
    //是否警告
    bool roiAlert, phoneAlert;
    //計算警報觸發開始和結束時間
    clock_t roiAlertBegin, roiAlertEnd;
    clock_t phoneAlertBegin, phoneAlertEnd;
    //紀錄開始計時與否
    bool roiTiming, phoneTiming;
    //根據事件發送Email, 設定寄信間隔時間
    void doSendEmailByEvent(const Mat sendFrame,const int timeInterval);
    //傳送Email給設定(參數1:附檔當下的frame, 參數2:事件, 參數3:發生時間)
    smtpSendEmail(const Mat cur_frame,const string event_type, const string occurTime);
    //SMTP的參數
    string smtpServer, smtpSender, smtpPassword, smtpReceiver;
    int smtpPort;
    //回傳檔案路徑(檔案名設為現在時間加上一個計數)
    string saveFilePath(const string DirPath,const int count);
    //印出Log訊息到TXT檔
    void Output_Log(const string messages);
    string logAddMem(const string messages);//將Log訊息加上此物件的記憶體位址
    //Log
    QString Log_Msg;
    string log_filename;
    outputLog *myLog;
    //Get Current Time
    string get_current_time();
    //Win Socket for sending the frame to the Server
    SOCKADDR_IN serverAddr;
    SOCKET      clientSock;
    int serverAddrLen;
    //Check ip alive
    QTcpSocket QSocket;
    //For update's
    QImage qimg;
    vector <RotatedRect> trackBox2_vector;
    //For estimate the FPS (Processing)
    int processTime,proc_sampleNumber,proc_fpsSum,proc_averageFPS;
    QTime proc_t;
    QQueue<int> proc_fps;
    void proc_updateFPS(int);
    //Mutex and WaitCondition
    QMutex processingMutex,doStopMutex,sync;
    QWaitCondition pauseCond;
    bool doStop;
    //Frame
    Mat cellphone_frame;//手機行為判斷的frame
    Mat send_frame;//寄送SMTP的frame
    // =========================== 以下都是思成學長的Code(有很多目前沒用到) ===============================
    //Video Src Addr File
    ifstream name_file;
    string fname,recordName;
    //counter
    int pop_counter,empty_counter,push_counter;
    int specialAlert = 0;
    int specialDriveAlert = 0;
    //Frame
    Mat frame_gray,frame_bg,frame_ref,frame_motion,frame_morph,frame_connected,frame_median,frame_median2,frame_ROI,frame_exitHuman,frame_exitVehicle,frame_specialArea;
    Mat frame_last,frame_diff,frame_diffbin,frame_diffmed,frame_diffmorph;
    Mat frame1,frame_clearbg;
    /* Camshift parameters */
    Mat image;
    bool backprojMode;
    int tx_min,ty_min,tx_max,ty_max;
    Rect selection;
    int hsize;
    float hranges[2] = {0,255};
    const int channels[3] = {0, 1, 2};
    const float cranges[2] = {0, 256};
    const float* phranges[3] = {cranges, cranges, cranges};
    Mat hsv,hue,mask;
    vector <Mat> histgrams;
    vector <trackingObject> objects;
    vector <trackingObject> oldObjects;
    vector <Rect> cars;
    vector <Rect> carsNotGetOff;
    int objCount;
    int breakCounter;
    int lastMovingNum;
    int breakCheck;
    Rect area = Rect(0,0,frameWidth,frameHeight);
    int textFontFace = FONT_HERSHEY_SIMPLEX;
    double textFontScale;
    int textThickness;
    const int h_bins = 32, s_bins = 8, v_bins = 2;
    const int histSize[3] = {h_bins, s_bins, v_bins};
    //File/IO
    VideoWriter writ;
    vector <Rect> ROIs;
    vector <Rect> exitHuman;
    vector <Rect> exitVehicle;
    vector <Rect> specialAreas;
    int temp_x,temp_y,temp_width,temp_height;
    fstream areaFile,vehicleFile,humanFile,ParkingLotFile,ParkingDisplayFile;

    /* Imgprocess Function */
    void test();
    Mat Grayscale(Mat original);
    bool BackgroundInitialize(VideoCapture &cap, Mat &bg, Mat &ref, Mat &last, vector <Rect> areas);
    void BackgroundUpdate(Mat* bg, Mat* ref, Mat gray, vector <Rect> areas);
    int MotionMask(Mat frame,Mat background,vector <Rect> areas);
    int FrameCompare(Mat frame,Mat background,CvRect area);
    void FrameRecord(string name, int num);
    void ConnectedComponents(IplImage *pImg,int areaThreshold);
    void icvprCcaByTwoPass(const cv::Mat& _binImg, cv::Mat& _lableImg);
    void FindBlobs(const cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs);
    int IntersectionCheck (Rect a, Rect b);
    int RecordObject (trackingObject obj, string rName,vector <Rect> exitH, vector <Rect> exitV);
    int checkEvent(trackingObject obj);
    int checkHuman(trackingObject obj);
    int checkPosition(Point pos, vector <Rect> exit);
    int pointRectDistance(Point a, Rect b);
    int isCovered(Rect a, Rect b);
    void drawRect(Mat& paint, Rect a, int g, int b, int r, int thick);
    void printMat (Mat p);
    void calcWeightedHistogram(Mat frame, const Rect roi, Mat &destHist, const Mat *mask = NULL);    //frame=hsv
    float kernelFunc(Point2f& roi_center, Point2f xy, int hx, int hy);
    void hist0To255(Mat &destHist);
    int checkThief(trackingObject obj,vector <Rect> cars);
};




#endif // ProcessThread_H
