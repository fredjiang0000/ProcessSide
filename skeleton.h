#ifndef SKELETON_H
#define SKELETON_H
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include <QThread>
#include <QObject>
#include <time.h>
#include <iostream>
#include "outputLog.h"
/*******************************************
 *              Define
 * BLOB_THRESHOLD 連通區塊Pixel數量(小於視為雜訊)
 * move_value 移動範圍，移動過少 手部點判斷錯誤，重新判斷
 * Human_Num 人物編號總量
 * Human_proportion 人物比例(頭部)
 * Cr : YCrCb 範圍
 * Cb : YCrCb 範圍
 * SkinClothes_proportion : 膚色衣服過濾比例
 * HumanRect :　人物框過濾比例
 * Rect_ADD　: 手部框增加大小
********************************************/
#define BLOB_THRESHOLD 300
#define move_value 5
#define Human_Num 10
#define Human_proportion 0.3
#define Cr_low 138
#define Cr_up 170
#define Cb_low 100
#define Cb_up 127
#define SkinClothes_proportion 15
#define HumanRect_High 4
#define HumanRect_Low 1.3
#define Rect_ADD 15
using namespace cv;
using namespace std;



class skeleton : public QThread
{
    Q_OBJECT
public:
    skeleton(Mat &img, Rect objrect, int human_num, int* timer);
    ~skeleton();
    vector<Mat> getImg();
    vector<Rect> getRect();
    void displaySkinMask(Mat frame);
    bool NoDetection();
protected:
    void run();
private:
    Mat img;
    Rect objrect;
    int human_num;
    int* timer;
    bool NoDetect;
    vector <Mat> pick_Img;
    vector <Rect> pick_Rect;
    CvRect GetHeadObjRect(Rect objrect);
    CvRect GetBodyObjRect(Rect objrect);
    CvPoint Head_skintracking(const Mat frame, Rect Head_rect, Vec3b &pixel,Rect &head_mask,int human_num);
    CvPoint Body_skintracking(const Mat frame,Rect Body_rect,int human_num);
    vector<Point> Hand_skintracking(const Mat frame, Rect objrect,Rect head_mask, vector<Rect> &pick_ROI, Vec3b pixel, int human_num);
    void pick_ROI(Mat &frame, Rect objrect, vector<Rect> Hand_ROI, int human_num,Point point[9], Rect area_head, Rect area_body);
    bool checkROI(Rect r, const Mat frame, int human_num);
    bool checkSkin(Rect r, const Mat frame);
    void initial(int human_num);
    Mat foremask(Mat img, Mat bgmask);
    bool Hand_shape_determinate(Mat oriframe, Point point[9], Mat frame, Rect ROI);
    bool KeyArea(Point point[9], Rect area_head, Rect area_body, Mat frame);
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
};
Rect rectSize(Mat img,Rect rect);
void FindBlobs(const Mat &binary, std::vector < std::vector<Point2i> > &blobs,long int Blob_size);
#endif // SKELETON_H
