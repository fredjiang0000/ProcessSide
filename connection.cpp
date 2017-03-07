#include "connection.h"
#include "ui_connection.h"
#include <iostream>
#include <QMessageBox>


using namespace std;

//=======================【連線設定】建構子========================
connection::connection(MainWindow* main_window,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::connection)
{
    myLog = new outputLog("connection.log");
    myLog->printLog(logAddMem("connection::connection() enter"));
    ui->setupUi(this);
    //初始化參數
    this->main_window = main_window;
    ui->hint->clear();
    vlc_thread = NULL;//VLC的Thread
    cap_thread = NULL;//Push的Thread(擷取影像串流)
    process_thread = NULL;//Pop的Thread(處理影像串流)
    concurrent_queue = NULL;//Frame Buffer
    isCamDisconnect = true;
    //update the mainwindow's Log
    QObject::connect(this,SIGNAL(PrintMyLog(QString,const int)),
                     main_window,SLOT(UpdateMyLog(QString,const int)));
    //Check IP CAM Alive, ping every ? msec
    ping_timer = new QTimer(this);
    QObject::connect(ping_timer, SIGNAL(timeout()),
                     this, SLOT(isIPAddressAlive()));
    myLog->printLog(logAddMem("connection::connection() exit"));
}
//===========【連線設定】解構子============
connection::~connection()
{
    myLog->printLog(logAddMem("connection::~connection() enter1"));
    delete ui;
    delete concurrent_queue;
    concurrent_queue = NULL;
    delete process_thread;
    process_thread = NULL;
    delete cap_thread;
    cap_thread = NULL;
    delete this->myLog;
    myLog = NULL;
    myLog->printLog(logAddMem("connection::~connection() enter2"));
}

//Close Event
void connection::closeEvent(QCloseEvent *event)
{
    myLog->printLog(logAddMem("connection::closeEvent()"));
    this->close();
}

//Show Event
void connection::showEvent(QShowEvent *event)
{
    myLog->printLog(logAddMem("connection::showEvent() enter"));
    ui->hint->clear();
    //讀取Network_profiles.txt中的連線設定參數
    Read_Network_profiles();
    myLog->printLog(logAddMem("connection::showEvent() exit"));
}

//取消按鈕
void connection::on_pushButton_cancel_clicked()
{
    myLog->printLog(logAddMem("connection::on_pushButton_cancel_clicked()"));
    this->close();
}

//確定按鈕
//目的:開啟串流網址,產生Push和Pop的Thread,將影像串流傳送到顯示端
void connection::on_pushButton_ok_clicked()
{
    myLog->printLog(logAddMem("connection::on_pushButton_ok_clicked() enter1"));
    ui->hint->setText("連線中......");
    ui->hint->show();
    this->repaint();
    //若之前的影像串流還在跑,則先停止之前的 Push&Pop Threads
    if(this->vlc_thread || this->cap_thread || this->process_thread)
        this->stopAllThreads();


    myLog->printLog(logAddMem("connection::on_pushButton_ok_clicked() enter6"));
    //**********從監視器的影像串流網址 切出IP + Port***********
    QString cam_ip_port;
    QString cam_ip;
    QString port;
    cam_ip_port = Get_Cam_IP_Port(ui->stream_address->text());
    cam_ip = Get_Cam_IP(cam_ip_port);
    port = Get_Cam_Port(cam_ip_port);
    qDebug() << "監視器RTSP串流:" << ui->stream_address->text();
    qDebug() << "監視器IP+Port:" << cam_ip_port ;
    qDebug() << "監視器IP:" << cam_ip;
    qDebug() << "Port:" << port;
    //*********從監視器的影像串流網址 切出IP + Port(End)********

    //Store ping ip and port to ping every ? secs(Check IP Cam Alive)
    this->ping_cam_ip = cam_ip;
    this->ping_cam_port = port.toInt();

    //******************* 區分RTSP串流 以及 HTTP串流網址 *****************
    //原因：OpenCV開啟RTSP的串流網址會有影像破格的問題,所以RTSP串流用VLC來開
    int length =  ui->stream_address->text().indexOf(":",0);
    string URL_Type = ui->stream_address->text().mid(0,length).toUtf8().constData();
    qDebug() << "URL_Type : " << QString::fromStdString(URL_Type);

    //******************* 區分RTSP串流 以及 HTTP串流網址(End) *****************

    //如果 是串流網址 則 檢查監視器的IP是否Ping的到
    if(URL_Type=="rtsp" || URL_Type=="http"){//用VLC播串流網址
        if(!isCameraIPAlive(ping_cam_ip,ping_cam_port))//
        {
            QMessageBox::critical(this,tr("錯誤"),tr("無法連線到監視器!\n請檢查監視器IP位址"));
            ui->hint->clear();
            return;
        }
        this->openVideo = false;
    }else{//用VLC播影片
        this->openVideo = true;
    }
    myLog->printLog(logAddMem("connection::on_pushButton_ok_clicked() enter7"));
    //檢查監視器的串流網址是否能打開,成功打開Set frame_width & height & FPS
    VideoCapture cap_temp;
    this->stream_addr = ui->stream_address->text().toUtf8().constData();//QString to std::string
    if(!connectToCamera(cap_temp,this->stream_addr))//開啟串流
        return;

    //儲存監視器地點、顯示端IP和Port
    this->server_ip_array = ui->server_ip->text().toUtf8();//(QString to QByteArray)
    this->location_array = ui->cam_location->text().toUtf8();//(QString to QByteArray)
    this->server_port = ui->server_port->text().toInt();//(QString to int)


    //Make a queue instance(接收Frame和處理Frame兩個Thread共用的Frame Buffer)
    if(!concurrent_queue)//如果為空才創一個新的
        concurrent_queue = new ConcurrentQueue<Mat>;
    //產生一個ProcessThread物件
    process_thread = new ProcessThread(main_window,concurrent_queue,//介面和Frame Buffer
                                       this->frame_width,this->frame_height,this->Cam_FPS,//監視器解析度寬、高和FPS
                                       this->location_array.data(),//監視器地點
                                       this->server_ip_array.data(),this->server_port);//Server's IP and Port

    cout << "process_thread created finished";
    //初始化設定偵測區域參數
    process_thread->initialization(cap_temp);
    cout << "process_thread initialization finished.";


    //New VlcThread and Initialize
    vlc_thread = new VlcThread(main_window,concurrent_queue,this->openVideo);
    //VLC Open Streaming and Initialization
    vlc_thread->VLC_Streaming_Init(this->stream_addr.c_str(),
                                   this->frame_width,this->frame_height);
    //========================= 連線至顯示端 ===================
    while(1)
    {
        if(process_thread->Connect_to_Server(process_thread->server_ip,process_thread->server_port))
            break;
        Sleep(1000);
    }
    vlc_thread->start(QThread::NormalPriority);//VLC Thread Start
    process_thread->start(QThread::HighPriority);//Processing Thread Start
    //成功連上監視器
    isCamDisconnect = false;
    //Check IP CAM Alive, ping every ? msec
    if(!openVideo)
        ping_timer->start(2000);
    myLog->printLog(logAddMem("connection::on_pushButton_ok_clicked() exit"));
    this->close();//關閉視窗
}

//檢查IP Camera是否斷線
//Input： 1.監視器的IP 2.監視器的Port
//Return： NULL
void connection::isIPAddressAlive()
{
    cout << "connection::isIPAddressAlive()" << endl;
    myLog->printLog(logAddMem("connection::isIPAddressAlive() enter1"));
    qDebug() << "檢查監視器連線狀態... IP:" << this->ping_cam_ip << " Port:" << this->ping_cam_port << endl;

    if(Qsocket.isOpen())
        Qsocket.close();
    //Ping監視器的IP位址(要加上Port)
    Qsocket.connectToHost(this->ping_cam_ip, this->ping_cam_port);
    //第一次Ping不到監視器IP位址
    if(!Qsocket.waitForConnected(7000)){
        //***********Output 斷線錯誤訊息*************
        qDebug() << "Error: " << Qsocket.errorString();
        cout << "IP Camera is disconnected. Cam's IP:" << this->ping_cam_ip.toStdString() << " Port:" << this->ping_cam_port <<
                " Error:" <<  Qsocket.errorString().toUtf8().constData()<< endl;
        myLog->printLog(logAddMem("connection::isIPAddressAlive() enter2(Error:IPCam is disconnected)"));
        //========清空傳給顯示端的Log,為了避免一直重連產生過多的訊息========
        if(process_thread->Log_for_display.size() > 10)
            process_thread->Log_for_display.clear();

        if(!isCamDisconnect){//第一次斷線才印錯誤訊息
            string log_msg = "IP Cam disconnected Error: "; log_msg += Qsocket.errorString().toUtf8().constData();
            log_msg = "監視器斷線!請重新檢查監視器的網路狀態";
            process_thread->Log_for_display.push_back(log_msg);
            log_msg = "connection::isIPAddressAlive() enter3(Error:";
            log_msg+=Qsocket.errorString().toUtf8().constData();  log_msg+=")";
            myLog->printLog(logAddMem(log_msg));
            Log_Msg = "監視器斷線!請重新檢查監視器的網路狀態";
            emit PrintMyLog(Log_Msg,2);
            this->stopAllThreads();//Stop All Threads
        }
        //***********Output 斷線錯誤訊息(End)*************
        isCamDisconnect = true;//紀錄斷線狀態
    }
    //Ping的到監視器IP位址
    else
    {
        //上一次連線正常,監視器正常連線
        if(!isCamDisconnect)
        {
            myLog->printLog(logAddMem("connection::isIPAddressAlive() enter4(IPCam is alive)"));
        }
        //【觸發條件】
        //上一次監視器網路斷線,當再度正常時(Ping的到),重新開啟Thread並連線到顯示端
        else
        {
            myLog->printLog(logAddMem("connection::isIPAddressAlive() enter5(Reconnect)"));
            Log_Msg = "重新開啟Thread並連線至顯示端...";
            emit PrintMyLog(Log_Msg,1);
            //重新開啟Thread並連線至顯示端
            if(Reconnection()==false)
            {
                myLog->printLog(logAddMem("connection::isIPAddressAlive() enter6(Error:Reconnect failed)"));
                Log_Msg = "Reconnection() 重新連線失敗!";
                emit PrintMyLog(Log_Msg,2);
            }
        }
    }
    //    ping_timer->start(2000);//啟動ping_timer
    myLog->printLog(logAddMem("connection::isIPAddressAlive() exit"));
}

//Output訊息至今天日期的Log檔
//void connection::Output_Log(const string messages)
//{
//    ofstream mylog;
//    //依照日期打開對應日期的txt檔
//    string log_filename = "Log "; log_filename += get_current_date(); log_filename += ".txt";
//    myLog->open(log_filename,ios::out | ios::app);
//    if(!mylog)
//    {
//        cout << "[Error] Fail to open MyLog"<< endl;
//        exit(1);
//    }
//    else
//    {
//        get_current_time(mylog);
//        mylog << "  " << ui->cam_location->text().toUtf8().constData() <<"   " << messages << endl;
//        myLog.close();
//    }
//}



//當處理端端重新Ping到監視器時,停止舊的Thread, 產生新的Thread, 重新連線至顯示端
bool connection::Reconnection()
{
    myLog->printLog(logAddMem("connection::Reconnection() enter1"));

    //檢查監視器的IP是否Ping的到
    if(!isCameraIPAlive(ping_cam_ip,ping_cam_port))
    {
        //========清空傳給顯示端的Log,為了避免一直重連產生過多的訊息========
        if(process_thread->Log_for_display.size() > 10)
            process_thread->Log_for_display.clear();
        string log_msg = "重新連線時,無法連上監視器!";
        //        Output_Log(log_msg);
        process_thread->Log_for_display.push_back(log_msg);
        Log_Msg = "重新連線時,無法連上監視器!";
        emit PrintMyLog(Log_Msg,2);
        myLog->printLog(logAddMem("connection::Reconnection() exit1"));
        return false;
    }
    myLog->printLog(logAddMem("connection::Reconnection() enter2"));
    //檢查監視器的串流網址是否能打開,成功打開Set frame_width&height
    VideoCapture cap_temp;
    if(!connectToCamera(cap_temp,this->stream_addr))//開啟串流
    {
        //========清空傳給顯示端的Log,為了避免一直重連產生過多的訊息========
        if(process_thread->Log_for_display.size() > 10)
            process_thread->Log_for_display.clear();
        string log_msg = "重新連線時,無法開啟監視器!";
        //        Output_Log(log_msg);
        process_thread->Log_for_display.push_back(log_msg);
        Log_Msg = "重新連線時,無法開啟監視器!";
        emit PrintMyLog(Log_Msg,2);
        myLog->printLog(logAddMem("connection::Reconnection() exit2"));
        return false;
    }
    myLog->printLog(logAddMem("connection::Reconnection() enter3"));

    //New A Mat Queue Buffer
    concurrent_queue = new ConcurrentQueue<Mat>;
    //產生一個ProcessThread物件
    process_thread = new ProcessThread(main_window,concurrent_queue,//介面和Frame Buffer
                                       frame_width,frame_height,Cam_FPS,//監視器解析度寬、高和FPS
                                       this->location_array.data(),//監視器地點
                                       this->server_ip_array.data(),this->server_port);//Server's IP and Port

    //初始化設定偵測區域參數
    process_thread->initialization(cap_temp);
    myLog->printLog(logAddMem("connection::Reconnection() enter4"));

    //如果是RTSP影像串流,則用VLC Player抓Frame,其他用OpenCV VideoCapture抓Frame
    //原因：RTSP影像串流用OpenCV VideoCapture抓影像會破格
    //New VlcThread and Initialize
    vlc_thread = new VlcThread(main_window,concurrent_queue,this->openVideo);
    //VLC Open Streaming and Initialization
    vlc_thread->VLC_Streaming_Init(this->stream_addr.c_str(),
                                   this->frame_width,this->frame_height);
    myLog->printLog(logAddMem("connection::Reconnection() enter5"));
    //========================= 連線至顯示端 ===================
    while(1)
    {
        if(process_thread->Connect_to_Server(process_thread->server_ip,process_thread->server_port))
            break;
        Sleep(1000);
    }
    myLog->printLog(logAddMem("connection::Reconnection() enter6"));
    //Start the Threads
    vlc_thread->start(QThread::NormalPriority);//VLC Thread Start
    process_thread->start(QThread::HighPriority);//Processing Thread Start
    //成功連上線
    isCamDisconnect = false;
    myLog->printLog(logAddMem("connection::Reconnection() exit4"));
    return true;
}


//從監視器影像串流網址切出IP+Port
QString connection::Get_Cam_IP_Port(QString rtsp_address)
{
    QString cam_ip_port;
    int begin = rtsp_address.indexOf("/",0) + 2;
    int end = rtsp_address.indexOf("/",7);
    int length = end - begin;
    cam_ip_port = rtsp_address.mid(begin,length);
    begin = cam_ip_port.indexOf("@");//特殊串流網址(有'@') ex:"http://andyunique:1234@192.168.1.33:554/track1"
    if(begin!=-1)
    {
        begin+= 1;
        end = cam_ip_port.length();
        length = end - begin;
        cam_ip_port = cam_ip_port.mid(begin,length);
    }
    return cam_ip_port;
}
//從IP:Port中切出IP
QString connection::Get_Cam_IP(QString cam_ip_port)
{
    //切出IP
    int begin = 0;
    int end = cam_ip_port.indexOf(":");
    int length = end - begin;
    QString cam_ip = cam_ip_port.mid(begin,length);
    return cam_ip;
}
//從IP:Port中切出Port
QString connection::Get_Cam_Port(QString cam_ip_port)
{
    QString port;
    //切出Port
    int begin = cam_ip_port.indexOf(":");
    int end  =  cam_ip_port.length();
    if(begin == -1)//如果沒有Port,預設為80
    {
        port = "80";
    }
    else
    {
        begin+=1;
        int length = end - begin;
        port =  cam_ip_port.mid(begin,length);
    }
    return port;
}
//檢查監視器的IP是否Ping的到
bool connection::isCameraIPAlive(const QString ping_ip, const int port)
{
    myLog->printLog(logAddMem("connection::isCameraIPAlive() enter1"));
    QTcpSocket QSocket;
    // this is not blocking call
    QSocket.connectToHost(ping_ip,port);
    // we need to wait...
    if(!QSocket.waitForConnected(3000))
    {
        qDebug() << "Error: " << QSocket.errorString();
        myLog->printLog(logAddMem("connection::isCameraIPAlive() exit1(Error:IP is dead)"));
        return false;
    }
    else
    {
        myLog->printLog(logAddMem("connection::isCameraIPAlive() exit2(IP is alive)"));
        return true;
    }
}

//檢查監視器的串流網址是否能打開,成功打開Set frame_width&height
//參數： 1.開串流物件 2.影像串流網址
//Return：是否成功開啟串流
bool connection::connectToCamera(VideoCapture &cap, const string stream_addr)
{
    myLog->printLog(logAddMem("connection::connectToCamera() enter1"));
    Mat temp_frame;
    // Open camera
    bool camOpenResult = cap.open(stream_addr);
    if(!camOpenResult)//開啟串流失敗,彈出錯誤視窗
    {
        QMessageBox::critical(0,tr("錯誤"),tr("無法開啟監視器串流!"));
        myLog->printLog(logAddMem("connection::connectToCamera() exit1(Error:Can't open Camera)"));
        cout << "Error opening IP Camera stream or file" << endl;
        return false;
    }else{ //成功Open Camera時,避免讀不到frame_width和frame_height,檢查讀不讀的到frame
        while(!cap.read(temp_frame))//讀不到則重新開起串流
        {
            myLog->printLog(logAddMem("connection::connectToCamera() enter2(Error:Can't read frame)"));
            cap.release();
            cap.open(stream_addr);
            continue;
        }
    }
    if(this->openVideo){
        this->Cam_FPS = cap.get(CV_CAP_PROP_FPS);//讀影片FPS
        if(this->Cam_FPS>60){//錯誤控制
            this->Cam_FPS = 30;
        }
    }
    else{
        // ===== 計算IP CAM的FPS(※使用get(CV_CAP_PROP_FPS)無法讀到正確的FPS)=====
        time_t start, end;
        int counter = 0;
        double sec;
        double fps;
        while(1){
            if(counter==0)
                time(&start);
            cap.read(temp_frame);
            time(&end);
            counter++;
            sec = difftime(end,start);
            fps = counter/sec;
            if(counter >= 90){
                //                printf("%.2f fps\n",fps);
                if(fps>60)
                    fps = 30;
                this->Cam_FPS = fps;
                break;
            }
        }
    }
    myLog->printLog(logAddMem("connection::connectToCamera() enter3"));
    // Set resolution and FPS
    this->frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    this->frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    string log_msg= "connection::connectToCamera() exit2(frame_width:";
    log_msg+= to_string(frame_width); log_msg+=" frame_height:"; log_msg+=to_string(frame_height);
    log_msg+= " Cam_FPS:"; log_msg+=to_string(Cam_FPS);

    cout << "Connection::frame_width : " << frame_width<< endl;
    cout << "Connection::frame_height : " << frame_height<< endl;
    cout << "Connection::Cam_FPS : " << Cam_FPS << endl;

    myLog->printLog(logAddMem(log_msg));
    // Return result
    return camOpenResult;
}
//讀取Network_profiles.txt中的連線設定參數
void connection::Read_Network_profiles()
{
    fstream fin;
    char line[100];
    fin.open("Network_profiles.txt",ios::in);
    int line_index = 0;
    while(fin.getline(line,sizeof(line),'\n')){
        if(line[0]!='#' && line[0]!='\0')//若讀到第一個字元為'#'或該行為空,則不進行設定
        {
            cout<<line<<endl;
            line_index ++;
            switch(line_index)
            {
            case 1:
                ui->cam_location->setText(line);break;
            case 2:
                ui->stream_address->setText(line);break;
            case 3:
                ui->server_ip->setText(line);break;
            case 4:
                ui->server_port->setText(line);break;

            default:
                break;
            }
        }
    }
    fin.close();
    cout << "Read Network_profiles.txt done."<< endl;
}

void connection::stopAllThreads()
{
    //Stop the Threads
    stopVLCThread();
    stopProcessingThread();
    //delete the Queue
    if(concurrent_queue)
    {
        delete concurrent_queue;
        concurrent_queue = NULL;
    }
}

void connection::delAllThreads()
{
    // Delete the Threads
    if(this->vlc_thread){
        delete this->vlc_thread;
        this->vlc_thread = NULL;
    }
    if(this->process_thread){
        delete this->process_thread;
        this->process_thread = NULL;
    }
    //delete the Queue
    if(concurrent_queue)
    {
        delete concurrent_queue;
        concurrent_queue = NULL;
    }
}

//Stop the Push&Pop Thread
void connection::stopCaptureThread()
{
    myLog->printLog(logAddMem("connection::stopCaptureThread() enter"));
    cap_thread->stop();
    //    concurrent_queue->WakeAllCondition();
    if(!cap_thread->wait(3000))
    {
        myLog->printLog(logAddMem("connection::stopCaptureThread() exit1(Can't Stop)"));
    }
    else
        myLog->printLog(logAddMem("connection::stopCaptureThread() exit2"));
}
//Stop the Processing(Pop) Thread
void connection::stopProcessingThread()
{
    myLog->printLog(logAddMem("connection::stopProcessingThread() enter"));
    process_thread->stop();
    concurrent_queue->WakeAllCondition();
    process_thread->wait();
    myLog->printLog(logAddMem("connection::stopProcessingThread() exit"));
}
//Stop the VLC(Only RTSP) Thread
void connection::stopVLCThread()
{
    myLog->printLog(logAddMem("connection::stopVLCThread() enter"));
    vlc_thread->stop();
    vlc_thread->wait();
    myLog->printLog(logAddMem("connection::stopVLCThread() exit"));
}

bool connection::isCamDisconnection()
{
    return isCamDisconnect;
}



string connection::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}


