#ifndef IMAGE_PROCESSING
#define IMAGE_PROCESSING

#define CAPTURE_FPS_STAT_QUEUE_LENGTH 32
//#define FRAME_WIDTH 1920
//#define FRAME_HEIGHT 1080
#define NUM_FRAME_PROCESS 3    //define how many frames to take one frame
#define NUM_FRAME_SAVE 15        //define the moving frame should also take how many prev & next frames
#define MORPH_SIZE 5
#define MORPH_ITERATION 5
#define MOVING_PIXEL_THRESHOLD 20
#define MOVING_OBJECT_THRESHOLD 0.2
#define SCALE_PARAMETER 20 // scale parameter b in the motion mask
#define BREAK_THRESHOLD 1 //���M�X�{�X�Ӳ��ʪ����H�W�����P�@�����}�H
#define BREAK_FRAMES_LIMIT 40 //�X�ӵe��(*3)���K�����D�}�H�����[�J
//tracking parameters
#define SMIN 0
#define VMIN 0
#define VMAX 255
#define BLOB_THRESHOLD 500         //面積幾個pixel以上才判斷為物件(否則視為雜訊濾除)    //影片4為100
#define INTERSECTION_THRESHOLD 30     //兩方法結果交集小於此值則判斷追蹤出了問題
#define TRACKWINDOW_THRESHOLD 1.5     //CAMSHIFT追蹤結果大於motion detection物件結果此倍數時則判斷追蹤出了問題
#define SAME_OBJECT_DISTANCE 10500    //兩物件距離小於此值時進行比較是否同物件之判斷
#define SAME_HIST_THRESHOLD 0.5 //50%
#define SAME_OBJECT_AREA_DIFFERENCE 200//平均物件大小相差幾個pixel數以內才可能為同物件
#define INTO_SCREEN_THRESHOLD 100    //每畫面變大多少pixel以上算進入中
#define DISSAPEAR_TIME  1500     //在 (DISSAPEAR_TIME/) 的時間內會檢查是否物件又出現

#define EVENT_TIME 125  //物件出現多少FRAME以上才會被視為事件紀錄
#define NOISE_HEIGHT_WIDTH_THRESHOLD 10  //平均長寬小於此值視為noise
#define HEIGHT_WIDTH_RATIO 1    //物件高寬比大於此值才可能是人
#define SIZE_THRESHOLD 20000     //物件平均大小小於此值才可能是人
#define SPEED_THRESHOLD 5000        //有某一秒的速度超過此值則非人 (此值為pixel移動數的平方/秒)
#define SPEED_LIMIT 10000   //某一個frame離上一個處理的frame物件距離超過此值則為追錯了
#define EXIT_DISTANCE 100   //距離畫面邊緣多近為離開/進入
#define NO_MOVE_THRESHOLD 10000  //一物件從出現到消失皆在多少距離內判斷為不動的東西(誤判)不記錄 (此值為pixel移動數的平方)
#define THIEF_DISTANCE 10000

#define PARKING 1
#define DRIVE_THROUGH 2
#define DRIVE_AWAY 3
#define CHANGE_PARKINGLOT 4
#define GET_INTO_CAR 5
#define WALK_THROUGH 6
#define GET_OFF_CAR 7
#define GO_BACK_TO_CAR 8


#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"
#include "opencv2/video/tracking.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <ctype.h>

using namespace std;
using namespace cv;

struct trackingObject
{
    int id; //物件編號
    Mat hist;   //物件Hue histgram
    //Mat histimg;
    Mat backproj;
    Rect trackWindow;
    vector <int> crossBlob;
    Rect previousBlob;
    vector <int> trajectoryTime;
    vector <Point> trajectoryPoint;
    vector <Rect> trajectoryRect;
    int sumArea;
    int sumHeight;
    int sumWidth;
    int overLap;
    int sumBlobs;
    int thief;
};


#endif
