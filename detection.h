#ifndef DETECTION_H
#define DETECTION_H

#define NUM_FRAME_PROCESS 3    //define how many frames to take one frame
#define SCALE_PARAMETER 20 // scale parameter b in the motion mask
#define MOVING_OBJECT_THRESHOLD 0.4

#include <opencv2/opencv.hpp>

#include <vector>

using namespace cv;
using namespace std;

class ObjectDetector{
    private:
        Mat back_frame;
        Mat ref_frame;
        Size dsize;
        vector<Rect> obj;
        int framecount;
        int min_obj_size;
        //float wh_ratio;
        //bool cut;
        vector<Rect> GetObjects(const Mat frame);
        double obDistance(const Rect a, const Rect b);
    public:
        Mat mask;
        ObjectDetector();
        //void setCut(bool s) { cut = s; }
        void setMinObjSize(int size) { min_obj_size = size; }
        //void setWhRatio(float ratio) { wh_ratio = ratio; }
        bool detect(Mat frame);
        bool backInit(VideoCapture &cap);
        void backInit(Mat frame);
        void backclear();
        void backUpdate(Mat gray);
        bool MotionMask(const Mat frame, Mat &motion_frame);
        vector<Rect> GetRect();

};

#endif
