#include "detection.h"

#include <qdebug.h>

ObjectDetector::ObjectDetector() : dsize(Size(640, 480)), framecount(0), min_obj_size(0)/*,  wh_ratio(0), cut(false)*/ {}

bool ObjectDetector::detect(Mat frame)
{
    Mat motion_frame;
    backUpdate(frame);

    //Motion detection
    if (MotionMask(frame, motion_frame)){
        motion_frame.copyTo(mask);
        //Object segmentaion
        vector<Rect> rects = GetObjects(mask);
        if (rects.size() > 0)
        {
            /*
            bool check = false;
            Rect select_rect,
                 center(frame.cols/2, frame.rows/2, 1, 1);
            //Set minimum distance to the center
            double MIN_distance = 100.0, temp = 0.0;
            //Get the nearest object to center
            for (unsigned int i = 0; i < rects.size(); ++i)
            {
                if ((temp = obDistance(rects[i], center)) < MIN_distance)
                {
                    select_rect = rects[i];
                    MIN_distance = temp;
                    check = true;
                }
            }
            //Check the width height ratio
            if (check && select_rect.height > select_rect.width*wh_ratio)
            {
                //Check the cut flag
                if (cut)
                    obj = Rect(select_rect.x, select_rect.y, select_rect.width, select_rect.height*7/9);
                else
                    obj = Rect(select_rect.x, select_rect.y, select_rect.width, select_rect.height);

                return true;
            }
            */
            obj = rects;
        }
    }
    return false;
}

bool ObjectDetector::backInit(VideoCapture &cap)
{
    if (!back_frame.empty())
        return false;
    const int START_AVERAGE = 30;
    Mat frame, frame_gray, *buffer = new Mat[START_AVERAGE];

    //Get frames
    for (int i = START_AVERAGE-1; i >= 0; --i){
        cap >> frame;
        if (frame.empty()){
            cerr << "Not enough frames\n";
            return false;
        }
        //flip(frame, frame, 0);
        resize(frame, frame, dsize);
        cvtColor(frame, frame_gray, CV_BGR2GRAY);
        frame_gray.copyTo(buffer[i]);
    }

    //Set the frame size
    buffer[0].copyTo(back_frame);
    buffer[START_AVERAGE-1].copyTo(ref_frame);

    int x = 0;
    int y = 0;
    int width = frame.cols;
    int height = frame.rows;
    int x_bound = x + width;
    int y_bound = y + height;

    //Initialize the background frame
    for(int i = y; i < y_bound; i++) {
        uchar* Bi = back_frame.ptr<uchar>(i);
        for(int j = x; j < x_bound; j++) {
            int counter = 0;
            for (int k = 0; k < START_AVERAGE; ++k){
                const uchar* Bufi = buffer[k].ptr<uchar>(i);
                counter += Bufi[j];
            }
            Bi[j] = counter/START_AVERAGE;
        }
    }
    delete [] buffer;
    return true;
}

void ObjectDetector::backInit(Mat frame)
{
    frame.copyTo(back_frame);
    frame.copyTo(ref_frame);
}

void ObjectDetector::backclear()
{
    back_frame.release();
    ref_frame.release();
    mask.release();
}

void ObjectDetector::backUpdate(Mat frame)
{
    if (++framecount < NUM_FRAME_PROCESS) //frameskip
        return;
    framecount = 0;
    Mat gray;
    cvtColor(frame, gray, CV_BGR2GRAY);
    resize(gray, gray, dsize);

    int x = 0;
    int y = 0;
    int width = gray.cols;
    int height = gray.rows;
    int x_bound = x + width;
    int y_bound = y + height;
    for(int i = y; i < y_bound; i++) {
        const uchar* Gi = gray.ptr<uchar>(i);
        uchar* Ri = ref_frame.ptr<uchar>(i);
        uchar* Bi = back_frame.ptr<uchar>(i);
        for(int j = x; j < x_bound; j++) {
            if (Ri[j] == Gi[j]) {   //Temporal match
                if (Bi[j] < Ri[j]) {    //Background modification
                    Bi[j] += min(Ri[j]-Bi[j] ,NUM_FRAME_PROCESS);
                }
                else {
                    Bi[j] -= min(Bi[j]-Ri[j] ,NUM_FRAME_PROCESS);
                }
            }
            else if (Ri[j] < Gi[j]) {    //Reference frame generation
                Ri[j] += min(Gi[j]-Ri[j] ,NUM_FRAME_PROCESS);
            }
            else {
                Ri[j] -= min(Ri[j]-Gi[j] ,NUM_FRAME_PROCESS);
            }
            // didn't do the Reference frame modification
        }
    }
}


bool ObjectDetector::MotionMask(const Mat frame, Mat &motion_frame)
{
    cvtColor(frame, motion_frame, CV_BGR2GRAY);

    resize(motion_frame, motion_frame, dsize);

    int x = 0;
    int y = 0;
    int width = motion_frame.cols;
    int height = motion_frame.rows;
    int x_bound = x + width;
    int y_bound = y + height;
    int sum = 0;
    int diff = 0;
    int histogram[256] = {0};

    for(int i = y; i < y_bound; i++) {
        const uchar* Fi = motion_frame.ptr<uchar>(i);
        const uchar* Bi = back_frame.ptr<uchar>(i);
        for(int j = x; j < x_bound; j++) {
            diff = abs(Fi[j] - Bi[j]);
            ++histogram[diff];
        }
    }

    //calculate a1 & a2 by Cauchy Distribution Model
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

    for(int i = y; i < y_bound; i++) {
        uchar* Fi = motion_frame.ptr<uchar>(i);
        const uchar* Bi = back_frame.ptr<uchar>(i);
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



    if ( (1.0 * sum / (width * height)) > MOVING_OBJECT_THRESHOLD ) return false;

    medianBlur(motion_frame, motion_frame, 5);
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(-1, -1));
    morphologyEx(motion_frame, motion_frame, MORPH_CLOSE, element, Point(-1, -1), 10);

    resize(motion_frame, motion_frame, frame.size());

    return true;

}

vector<Rect> ObjectDetector::GetRect()
{
    return obj;
}

vector<Rect> ObjectDetector::GetObjects(const Mat frame)
{
    //imshow("MASK ", frame);
    Mat temp;
    frame.copyTo(temp);
    vector<Rect> Rects;

    vector< vector<Point> > contours;
    findContours(temp, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    vector< vector<Point> >::const_iterator itContours = contours.begin();
    for(;itContours != contours.end();++itContours){
        Rect r = boundingRect(*itContours);
        if (r.width*r.height > min_obj_size){
            Rects.push_back(r);
        }
    }

    return Rects;
}

double ObjectDetector::obDistance(const Rect a, const Rect b)
{
    int ax, ay, bx, by;
    ax = a.x+a.width/2;
    ay = a.y+a.height/2;
    bx = b.x+b.width/2;
    by = b.y+b.height/2;
    return sqrt((double)((ax-bx)*(ax-bx)+(ay-by)*(ay-by)));
}
