#include "skeleton.h"
using namespace cv;
using namespace std;
/*******************************************
                Grobal
*******************************************/
int L_check=0;
int R_check=0;
int checker[Human_Num];
int false_checker[Human_Num];
/******************************************/
vector<Point> Head_record[Human_Num];
vector<Point> Body_record[Human_Num];
vector<Point> L_Hand_Record[Human_Num];
vector<Point> R_Hand_Record[Human_Num];
vector<Rect> Head_mask[Human_Num];

outputLog skeletonLog("skeleton.log");
/********************************************
*               建構子
********************************************/
skeleton::skeleton(Mat &img, Rect objrect, int human_num, int* timer)
{
    skeletonLog.printLog(logAddMem("skeleton::skeleton() enter"));
    this->img = img;
    this->objrect = objrect;
    this->human_num = human_num;
    this->timer = timer;
    QObject::connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
//    namedWindow("skinColor Windows",WINDOW_NORMAL);
    skeletonLog.printLog(logAddMem("skeleton::skeleton() exit"));
}
/********************************************
*               解構子
********************************************/
skeleton::~skeleton()
{
    skeletonLog.printLog(logAddMem("skeleton::~skeleton()"));
}
/*******************************************
 * 〔Function〕骨架 Main
 * img : orignal frame
 * objrect : rect of human
********************************************/
void skeleton::run(){
    skeletonLog.printLog(logAddMem("skeleton::run() enter"));
    NoDetect = true;
    if(checkROI(objrect,img,human_num)==false || checkSkin(objrect,img)==false  ){
        NoDetect = false;
        skeletonLog.printLog(logAddMem("skeleton::run() exit1"));
        return ;
    }
    Mat process_img;
    img.copyTo(process_img);
    Point point[9]={Point(0,0)};
    Vec3b pixel;
/*******************取得ROI*********************************/
    Rect head_rect = GetHeadObjRect(objrect);
    Rect body_rect = GetBodyObjRect(objrect);
/*******************取得POINT*******************************
 * 0 : Body Point
 * 1 : Neck Point
 * 2 : Head Point
 * 3 : Left Leg point
 * 4 : Right Leg Point
 * 5 : Left Arm Point
 * 6 : Right Arm Point
 * 7 : Left Hand Point
 * 8 : Right Hand Point
/***********************************************************/
    Rect head_mask;
    point[0] = Body_skintracking(process_img,objrect,human_num);
    point[2] = Head_skintracking(process_img,head_rect,pixel,head_mask,human_num);
    if(point[2].x == 0 || point[2].y == 0){
        NoDetect = false;
        skeletonLog.printLog(logAddMem("skeleton::run() exit2"));
        return ;
    }
    point[1] = Point((point[0].x/4+point[2].x*3/4),(point[0].y/4+point[2].y*3/4));
    point[5] = Point((point[0].x/3+point[2].x*2/3)-objrect.width/4,point[1].y);
    point[6] = Point((point[0].x/3+point[2].x*2/3)+objrect.width/4,point[1].y);
    point[3] = Point(point[5].x,point[0].y+objrect.height/4);
    point[4] = Point(point[6].x,point[0].y+objrect.height/4);
    vector<Rect> Hand_ROI;
    vector <Point> Hand_Point = Hand_skintracking(process_img,objrect,head_mask,Hand_ROI,pixel,human_num);
    point[7] = Hand_Point[0];
    point[8] = Hand_Point[1];
    head_mask.x += objrect.x;
    head_mask.y += objrect.y;

    /**************************顯示骨架******************************************
    ellipse(img,point[0],cvSize(3,3),0,0,360.0,CV_RGB(255,255,0),3,8,0);
    ellipse(img,point[2],cvSize(3,3),0,0,360.0,CV_RGB(255,255,0),3,8,0);
    ellipse(img,point[1],cvSize(3,3),0,0,360.0,CV_RGB(255,255,0),3,8,0);
    ellipse(img,point[5],cvSize(3,3),0,0,360.0,CV_RGB(255,255,0),3,8,0);
    ellipse(img,point[6],cvSize(3,3),0,0,360.0,CV_RGB(255,255,0),3,8,0);
    ellipse(img,point[3],cvSize(3,3),0,0,360.0,CV_RGB(255,255,0),3,8,0);
    ellipse(img,point[4],cvSize(3,3),0,0,360.0,CV_RGB(255,255,0),3,8,0);
    line(img,point[2],point[1],CV_RGB(0,0,255),3);
    line(img,point[1],point[0],CV_RGB(0,0,255),3);
    line(img,point[1],point[5],CV_RGB(0,0,255),3);
    line(img,point[1],point[6],CV_RGB(0,0,255),3);
    line(img,point[0],point[3],CV_RGB(0,0,255),3);
    line(img,point[0],point[4],CV_RGB(0,0,255),3);
    line(img,Point(point[3].x,objrect.y+objrect.height),point[3],CV_RGB(0,0,255),3);
    line(img,Point(point[4].x,objrect.y+objrect.height),point[4],CV_RGB(0,0,255),3);
    if(point[7].x > objrect.x && point[7].x < objrect.x+objrect.width){
        ellipse(img,point[7],cvSize(10,10),0,0,360.0,CV_RGB(255,255,0),3,8,0);
        line(img,point[5],point[7],CV_RGB(0,0,255),3);
    }
    if(point[8].x > objrect.x && point[8].x < objrect.x+objrect.width){
        ellipse(img,point[8],cvSize(10,10),0,0,360.0,CV_RGB(255,255,0),3,8,0);
        line(img,point[6],point[8],CV_RGB(0,0,255),3);
    }
    for(int i = 0 ; i < (int)Hand_ROI.size();i++){
        // rectangle(img,Hand_ROI[i],Scalar(0,0,255),3);
    }
    ************************************************************************/
    for(int i =0 ;i< (int)Hand_ROI.size() ;i++){
        Rect temp_ROI = Hand_ROI[i];
        temp_ROI.x = temp_ROI.x-10;
        temp_ROI.y = temp_ROI.y-10;
        temp_ROI.width = temp_ROI.width+20;
        temp_ROI.height = temp_ROI.height+20;
        temp_ROI = rectSize(img,temp_ROI);
        rectangle(img,temp_ROI,CV_RGB(255,255,0),5);
    }
    int key_rect[Human_Num]={0};
    /********************判斷手部點是否處於關鍵區域***********************/
    Rect area_head = Rect( objrect.x,head_mask.y, objrect.width, head_mask.height);
    Rect area_body = Rect( point[5].x,point[1].y, point[6].x-point[5].x,point[0].y-point[1].y);
    if(KeyArea(point,area_head,area_body,img)){
        key_rect[human_num]++;
    }
    /***********************調整靈敏度*********************************/
    if( key_rect[human_num] > 0 ){
        checker[human_num]++;
    }
    else{
        false_checker[human_num]++;
    }
    if(false_checker[human_num]> (*timer-1)){
        checker[human_num]=0;
        false_checker[human_num] = 0;
    }
    /******************************************************************/
    if(checker[human_num] > (*timer-1) && key_rect[human_num]>0 ){ //調整靈敏度
        pick_ROI(process_img,objrect,Hand_ROI,human_num,point,area_head,area_body);
        checker[human_num] = 0;
        false_checker[human_num] = 0;
        for(int i =0 ;i< (int)Hand_ROI.size() ;i++){
            Rect temp_ROI = Hand_ROI[i];
            temp_ROI.x = temp_ROI.x-10;
            temp_ROI.y = temp_ROI.y-10;
            temp_ROI.width = temp_ROI.width+20;
            temp_ROI.height = temp_ROI.height+20;
            temp_ROI = rectSize(img,temp_ROI);
            rectangle(img,temp_ROI,CV_RGB(255,255,0),5);
        }

    }
    skeletonLog.printLog(logAddMem("skeleton::run() exit3"));
    return ;
}
/*******************************************
〔Function〕取得頭部區塊
objrect : rect of human
********************************************/
CvRect skeleton::GetHeadObjRect(Rect objrect)
{
    skeletonLog.printLog(logAddMem("skeleton::GetHeadObjRect() enter"));
    Rect temp; // 紀錄頭部區塊的矩形
    // 由人物比例推得 (x,y = 人物區塊的x ,y  & width = 人物區塊的width & Height = 人物區塊的height*0.3 )
    temp = Rect(objrect.x,objrect.y,objrect.width,objrect.height*Human_proportion);
    skeletonLog.printLog(logAddMem("skeleton::GetHeadObjRect() exit"));
    return temp;
}
/*******************************************
〔Function〕取得身體區塊
objrect : rect of human
********************************************/
CvRect skeleton::GetBodyObjRect(Rect objrect)
{
    skeletonLog.printLog(logAddMem("skeleton::GetBodyObjRect() enter"));
    Rect temp;// 紀錄身體區塊的矩型
    // 由人物比例推得 (x,y = 人物區塊的x ,y-頭部區塊的height & width = 人物區塊的width & Height = 人物區塊的height*0.7 )
    temp = Rect(objrect.x,objrect.y+objrect.height*Human_proportion,objrect.width,objrect.height*(1-Human_proportion));
    skeletonLog.printLog(logAddMem("skeleton::GetBodyObjRect() exit"));
    return temp;
}
/*******************************************
 *〔Function〕頭部膚色追蹤，並取得頭部點
 * 做法 : 對頭部區塊作膚色判斷
 * frame : orignal frame
 * Head_rect : rect of head
 * pixel : skin pixel of head
********************************************/
CvPoint skeleton::Head_skintracking(const Mat frame,Rect Head_rect,Vec3b &pixel,Rect &head_mask,int human_num)
{
    skeletonLog.printLog(logAddMem("skeleton::Head_skintracking() enter"));
    if((Head_rect.y >0 || Head_rect.x >0  )&&(Head_rect.y <frame.rows || Head_rect.x <frame.cols  )) //判斷頭部區塊是否錯誤
    {
        Mat Head_ROI;
        Mat YCrCb,Gray;
        Head_rect=rectSize(frame,Head_rect); // 檢查區塊大小有無大於圖片大小
        cvtColor(frame(Head_rect),YCrCb,CV_BGR2YCrCb); // BGR -> YCrCb
        int skin_counter=0;
        for(int i=0 ; i < YCrCb.rows ;i++ ){
            for(int j=0 ; j < YCrCb.cols ; j++ ){
                Vec3b Pixel = YCrCb.at<Vec3b>(i,j);//YCrCb pixel
                if(Pixel[1] >= Cr_low && Pixel[1] <= Cr_up && Cb_low <= Pixel[2] && Pixel[2] <= Cb_up){
                    YCrCb.at<Vec3b>(i,j)[0]=255;
                    YCrCb.at<Vec3b>(i,j)[1]=255;
                    YCrCb.at<Vec3b>(i,j)[2]=255;
                    skin_counter++;
                }
                else{
                    YCrCb.at<Vec3b>(i,j)[0]=0;
                    YCrCb.at<Vec3b>(i,j)[1]=0;
                    YCrCb.at<Vec3b>(i,j)[2]=0;
                }
            }
        }
        /****************頭部區塊膚色數量判斷****************************/
        if(0.07 >= (double)skin_counter/(Head_rect.height*Head_rect.width)){
            skeletonLog.printLog(logAddMem("skeleton::Head_skintracking() exit1"));
            return Point(0,0);
        }
        /************************************************************/
        cvtColor(YCrCb,Gray,CV_RGB2GRAY); //RGB->GRAY
        medianBlur(Gray,Gray,5);//中值濾波
        Mat element = getStructuringElement( 2, Size( 5,5), Point( 2,2 ) ); //宣告一元素，用於型態學處理
        morphologyEx(Gray,Gray, MORPH_CLOSE, element , Point( -1,-1 ) , 3);//型態學
        dilate(Gray,Gray,element);
        vector < vector<Point2i > > blobs; // 紀錄連通圖區塊
        FindBlobs(Gray,blobs,BLOB_THRESHOLD); // 連通圖Function
        Mat skin = Mat::zeros(Gray.size(), CV_8UC3);
        size_t temp_size=0;
        int first = -1 ;
        for(size_t i=0; i < blobs.size(); i++) {
            if(temp_size < blobs[i].size()){
                temp_size = blobs[i].size();
                first = i;
            }
        }
        Rect temp_ROI;
        int sum_x=0,sum_y=0;
        int max_x=0,max_y=0;
        int min_x=10000,min_y=10000;
        if(first != -1){
            for(size_t j=0; j < blobs[first].size(); j++) {
                int x = blobs[first][j].x;
                int y = blobs[first][j].y;
                sum_x += blobs[first][j].x;
                sum_y += blobs[first][j].y;
                if(max_x < x)
                    max_x = x;
                if(max_y < y)
                    max_y = y;
                if(min_x > x)
                    min_x = x;
                if(min_y > y)
                    min_y = y;
            }
            head_mask.x=min_x;
            head_mask.y=min_y;
            head_mask.width=max_x-min_x;
            head_mask.height=max_y-min_y;
            Point head_Point = Point((sum_x/blobs[first].size())+Head_rect.x,(sum_y/blobs[first].size())+Head_rect.y);
            Head_record[human_num].push_back(head_Point); // 紀錄頭部參考資訊

            if( (head_mask.width * head_mask.height ) <= (Head_rect.height * Head_rect.width)/2){
                head_mask.x = head_Point.x - Head_rect.x - Head_rect.width/4;
                head_mask.y = 0;
                head_mask.width = Head_rect.width/2;
                head_mask.height =  Head_rect.height;
            }
            skeletonLog.printLog(logAddMem("skeleton::Head_skintracking() exit2"));
            return head_Point;
        }
        else{
            skeletonLog.printLog(logAddMem("skeleton::Head_skintracking() exit3"));
            return Point(0,0);
        }
     }
     else{
         if(Head_record[human_num].size()>0){//判斷頭部點參考資訊是否存在
             skeletonLog.printLog(logAddMem("skeleton::Head_skintracking() exit4"));
             return Head_record[human_num].back();
         }
         else{
             skeletonLog.printLog(logAddMem("skeleton::Head_skintracking() exit5"));
             return Point(0,0);//沒有參考資訊，回傳Point (0,0)
         }
     }
    skeletonLog.printLog(logAddMem("skeleton::Head_skintracking() exit6"));
}
/*******************************************
*〔Function〕取得身體點
* frame : orignal frame
* Body_rect : rect of body
********************************************/
CvPoint skeleton::Body_skintracking(const Mat frame, Rect Body_rect, int human_num){
    skeletonLog.printLog(logAddMem("skeleton::Body_skintracking() enter"));
    Point body_point; // 紀錄身體中心點
    if((Body_rect.y >0 || Body_rect.x >0  )&&(Body_rect.y <frame.rows || Body_rect.x <frame.cols  )){ //檢查身體曲塊是否錯誤
        body_point = Point(Body_rect.x+Body_rect.width/2,Body_rect.y+Body_rect.height/2);//計算身體中心點
        Body_record[human_num].push_back(body_point);//紀錄身體中心點參考資訊
    }
    else{
        body_point = Body_record[human_num].back();// 回傳紀錄的身體中心點

    }
    skeletonLog.printLog(logAddMem("skeleton::Body_skintracking() exit"));
    return body_point;
}
/*******************************************
* 〔Function〕手部膚色追蹤，並取得手部點
* 做法 : 對身體部分作膚色判斷
* frame : orignal frame
* objrect : rect of body
* pixel : skin pixel of head (reference)
********************************************/
vector<Point> skeleton::Hand_skintracking(const Mat frame,Rect objrect,Rect head_mask,vector<Rect> &pick_ROI,Vec3b pixel,int human_num){
    skeletonLog.printLog(logAddMem("skeleton::Hand_skintracking() enter"));
    vector<Point> point; //紀錄雙手點
    //雙手點初始值
    Point a = Point(0,0);
    Point b = Point(0,0);
    point.push_back(a);
    point.push_back(b);
    Mat img_ROI;

    cvtColor(frame(objrect),img_ROI,CV_BGR2YCrCb); //BGR-> HSV
    /********************檢查膚色區塊**************************/
    for(int i=0;i<img_ROI.rows;i++){
        for(int j=0;j<img_ROI.cols;j++){
            Vec3b Pixel =img_ROI.at<Vec3b>(i,j);
            if(i >= head_mask.y && i <= head_mask.y+head_mask.height && j >= head_mask.x && j <= head_mask.x+head_mask.width  ){
                img_ROI.at<Vec3b>(i,j)[0]=0;
                img_ROI.at<Vec3b>(i,j)[1]=0;
                img_ROI.at<Vec3b>(i,j)[2]=0;
            }
            else if(Pixel[1] >= Cr_low && Pixel[1] <= Cr_up && Cb_low <= Pixel[2] && Pixel[2] <= Cb_up){
                img_ROI.at<Vec3b>(i,j)[0]=255;
                img_ROI.at<Vec3b>(i,j)[1]=255;
                img_ROI.at<Vec3b>(i,j)[2]=255;
            }
            else{
                img_ROI.at<Vec3b>(i,j)[0]=0;
                img_ROI.at<Vec3b>(i,j)[1]=0;
                img_ROI.at<Vec3b>(i,j)[2]=0;
            }

        }
    }
    /*******************形態學處理**********************************/
    cvtColor(img_ROI,img_ROI,CV_RGB2GRAY); //RGB->GRAY
    medianBlur(img_ROI,img_ROI,5);//中值濾波
    Mat element = getStructuringElement( 2, Size( 5,5), Point( 2,2 ) ); //宣告一元素，用於型態學處理
    morphologyEx(img_ROI,img_ROI, MORPH_CLOSE, element , Point( -1,-1 ) , 3);//型態學
    dilate(img_ROI,img_ROI,element);
    /*******************************************************************
    char temp[50];
    time_t t;
    struct tm *T;
    time(&t);
    T = localtime(&t);
    sprintf(temp,"cell_phone2/image_%dDay_%dHours_%dMin_%dSec.jpg",T->tm_mday,T->tm_hour,T->tm_min,T->tm_sec);
    imwrite(temp,img_ROI);
    *******************************************************************/
    vector < vector<Point2i > > blobs; // 紀錄連通圖區塊
    FindBlobs(img_ROI,blobs,BLOB_THRESHOLD); // 連通圖Function
    Mat skin = Mat::zeros(img_ROI.size(), CV_8UC3);
    size_t temp_size=0;
    int first,second;
    vector<int> pick;
    /******************找最大跟第二大**************************************/
    if(blobs.size() >= 2){
        for(size_t i=0; i < blobs.size(); i++) {
            if(temp_size < blobs[i].size()){
                temp_size = blobs[i].size();
                first = i;
            }
        }
        temp_size=0;
        for(size_t i=0; i < blobs.size(); i++) {
            if(i != first){
                if(temp_size < blobs[i].size()){
                    temp_size = blobs[i].size();
                    second = i;
                }
            }
        }
        pick.push_back(first);
        pick.push_back(second);
    }
    else{
        for(size_t i=0; i < blobs.size(); i++) {
            if(temp_size < blobs[i].size()){
                temp_size = blobs[i].size();
                first = i;
            }
        }
        pick.push_back(first);
    }
    /**********************標示各聯通圖****************************/
    Rect temp_ROI;
    for(int k = 0;k<pick.size();k++){
        for(size_t i=0; i < blobs.size(); i++){
            if(i==pick[k]){
                int sum_x=0,sum_y=0;
                int max_x=0,max_y=0;
                int min_x=10000,min_y=10000;
                for(size_t j=0; j < blobs[i].size(); j++) {
                    int x = blobs[i][j].x;
                    int y = blobs[i][j].y;
                    if(x > 10 &&y >10){
                        sum_x += blobs[i][j].x;
                        sum_y += blobs[i][j].y;
                    }
                    if(max_x < x)
                        max_x = x;
                    if(max_y < y)
                        max_y = y;
                    if(min_x > x)
                        min_x = x;
                    if(min_y > y)
                        min_y = y;

                }
                Point temp = Point((sum_x/blobs[i].size())+objrect.x,(sum_y/blobs[i].size())+objrect.y);
                if(temp.x != objrect.x && temp.y != objrect.y ){
                    point[k]=temp;
                    temp_ROI.x=min_x+objrect.x;
                    temp_ROI.y=min_y+objrect.y;
                    temp_ROI.width=max_x-min_x;
                    temp_ROI.height=max_y-min_y;
                    pick_ROI.push_back(temp_ROI);
                }
            }
        }
    }


//    imshow("skinColor Windows",img_ROI);
//    waitKey(1);
    skeletonLog.printLog(logAddMem("skeleton::Hand_skintracking() exit"));
    return point;
}
/*******************************************
* 〔Function〕關鍵區域判斷，並擷出手部區塊
* img : orignal frame
* objrect : rect of body
* pixel : skin pixel of head (reference)
********************************************/
void skeleton::pick_ROI(Mat &frame,Rect objrect,vector<Rect> Hand_ROI,int human_num,Point point[9],Rect area_head,Rect area_body){
    skeletonLog.printLog(logAddMem("skeleton::pick_ROI() enter"));
    for(int i =0 ;i< (int)Hand_ROI.size() ;i++){
        if(((point[i+7].x > area_head.x && point[i+7].x < area_head.x + area_head.width
            && point[i+7].y > area_head.y && point[i+7].y < area_head.y+area_head.height)||//頭中心
            (( point[i+7].x > area_body.x && point[i+7].x < area_body.x+area_body.width)
            && point[i+7].y > area_body.y && point[i+7].y < area_body.y+area_body.height)) //身體中心
            && point[i+7].y !=0 ){ //判斷手勢是否在關鍵區域
            Rect temp_ROI = Hand_ROI[i];
            temp_ROI.x = temp_ROI.x-Rect_ADD;
            temp_ROI.y = temp_ROI.y-Rect_ADD;
            temp_ROI.width = temp_ROI.width+Rect_ADD*2;
            temp_ROI.height = temp_ROI.height+Rect_ADD*2;
            temp_ROI = rectSize(frame,temp_ROI);
            bool hand_shape = Hand_shape_determinate(frame(objrect),point,frame(temp_ROI),temp_ROI);
            //cout << "hand_shape" << hand_shape<<endl;
            if(hand_shape == true){
                pick_Img.push_back(frame(temp_ROI));
                pick_Rect.push_back(temp_ROI);
            }
        }
    }
    skeletonLog.printLog(logAddMem("skeleton::pick_ROI() exit"));
}
/*******************************************
 * 〔Function〕判斷人物區塊長寬比是否符合1:3
 *  做法 :　判斷區塊是否長寬比是否符合1:3
 *  r : rect of frame
 *  frame : frame
********************************************/
bool skeleton::checkROI(Rect r,const Mat frame,int human_num){
    skeletonLog.printLog(logAddMem("skeleton::checkROI() enter"));
    if(r.height/r.width > HumanRect_High || r.height/r.width < HumanRect_Low){//人物框的長寬比不得超過限制範圍
        skeletonLog.printLog(logAddMem("skeleton::checkROI() exit1"));
        return false;
    }
    if(frame.rows*3/4 < r.height){//人物框的高不能超過總螢幕的四分之三
        skeletonLog.printLog(logAddMem("skeleton::checkROI() exit2"));
        return false;
    }
    /*
    if(r.x < 30 || r.y < 30 && r.x+r.width > frame.cols-30 || r.y+r.height > frame.rows-30){
        return false;
    }
    */
    skeletonLog.printLog(logAddMem("skeleton::checkROI() exit3"));
    return true;
}
/********************************************
 * 〔Function〕判斷人物區塊中Skin pixel 是否過多
 *  做法 :　判斷區塊中Skin pixel 是否過多
 *  r : rect of frame
 *  frame : frame
 *
*********************************************/
bool skeleton::checkSkin(Rect r, const Mat frame){
    skeletonLog.printLog(logAddMem("skeleton::checkSkin() enter"));
    Mat YCrCb;
    cvtColor(frame,YCrCb,CV_BGR2YCrCb);
    long int check_size=0;
    for(int i = r.y ; i < r.y+r.height ; i++){
        for(int j =r.x ; j < r.x+r.width ; j++){
            Vec3b Pixel =YCrCb.at<Vec3b>(i,j);
            if(Pixel[1] >= Cr_low && Pixel[1] <= Cr_up && Cb_low <= Pixel[2] && Pixel[2] <= Cb_up){
                check_size++;
            }
        }
    }
    if(check_size <= r.width * r.height/SkinClothes_proportion ){//過濾膚色太多的情形 ex:穿膚色衣服
        skeletonLog.printLog(logAddMem("skeleton::checkSkin() exit1"));
        return true;
    }
    else{
        skeletonLog.printLog(logAddMem("skeleton::checkSkin() exit2"));
        return false;
    }
}
/********************************************
 * 〔Function〕 判斷是關鍵區域
 *
********************************************/
bool skeleton::KeyArea(Point point[9],Rect area_head,Rect area_body,Mat frame){
    skeletonLog.printLog(logAddMem("skeleton::KeyArea() enter"));
    for(int i=7;i < 9;i++){
        if(((point[i].x > area_head.x && point[i].x < area_head.x + area_head.width
            && point[i].y > area_head.y && point[i].y < area_head.y+area_head.height)||//頭中心
            (( point[i].x > area_body.x && point[i].x < area_body.x+area_body.width)
            && point[i].y > area_body.y && point[i].y < area_body.y+area_body.height)) //身體中心
            && point[i].y !=0 ){ //判斷手勢是否在關鍵區域
            //rectangle(img,area_head,Scalar(0,0,255),3);
            //rectangle(img,area_body,Scalar(0,0,255),3);
            //ellipse(img,point[7],cvSize(5,5),0,0,360.0,CV_RGB(255,255,0),5,8,0);
            //ellipse(img,point[8],cvSize(5,5),0,0,360.0,CV_RGB(255,255,0),5,8,0);
            skeletonLog.printLog(logAddMem("skeleton::KeyArea() exit1"));
            return true;
        }
    }
    skeletonLog.printLog(logAddMem("skeleton::KeyArea() exit2"));
    return false;
}


/************************************
 * 〔Function〕 圖片區塊錯誤，重新修改範圍
 * img : 原圖
 * rect : 要判斷的rect
************************************/
Rect rectSize(Mat img,Rect rect){
     skeletonLog.printLog("skeleton thread rectSize() enter");
    if(rect.x < 0 ){
        rect.x = 0;
    }
    if(rect.y < 0 ){
        rect.y = 0;
    }
    if(rect.x > img.cols-rect.width){
        rect.width = img.cols-rect.x;
    }
    if(rect.y > img.rows-rect.height){
        rect.height = img.rows - rect.y;
    }
    skeletonLog.printLog("skeleton thread rectSize() exit");
    return rect;
}
/****************************************
 *〔Function〕初始化紀錄
 * human_num 人物編號
****************************************/
void skeleton::initial(int human_num){
    skeletonLog.printLog(logAddMem("skeleton::initial() enter"));
    Head_record[human_num].clear();
    Body_record[human_num].clear();
    L_Hand_Record[human_num].clear();
    R_Hand_Record[human_num].clear();
    skeletonLog.printLog(logAddMem("skeleton::initial() exit"));
    return ;
}
/*******************************************
* 〔Function〕連通圖
* binary : Gray image
* blob : connect component
* blob_size : if blobsize < blob_size  == noise
********************************************/
void FindBlobs(const Mat &binary, std::vector < std::vector<Point2i> > &blobs,long int Blob_size)
{
    skeletonLog.printLog("skeleton thread FindBlobs() enter");
    blobs.clear(); // 初始化
    // Fill the label_image with the blobs
    // 0  - background
    // 1  - unlabelled foreground
    // 2+ - labelled foreground

    Mat label_image;
    binary.copyTo(label_image);
    threshold(binary, label_image, 0, 1, THRESH_BINARY);
    //binary.copyTo(label_image);
    int label_count = 2; // starts at 2 because 0,1 are used already

    //??floodfill??(opencv)??????
    for(int y=0; y < label_image.rows; y++) {
        uchar *row = label_image.ptr<uchar>(y);
        for(int x=0; x < label_image.cols; x++) {
            if(row[x] != 1) {
                continue;
            }
            Rect rect;
            floodFill(label_image, Point(x,y), label_count, &rect, 0, 0, 8); //(功能) 連通圖著色 - 小畫家倒油漆

            std::vector <Point2i> blob;

            for(int i=rect.y; i < (rect.y+rect.height); i++) {
                uchar *row2 = label_image.ptr<uchar>(i);
                for(int j=rect.x; j < (rect.x+rect.width); j++) {
                    if(row2[j] != label_count) {
                        continue;
                    }

                    blob.push_back(Point2i(j,i));
                }
            }
            if ((unsigned int)blob.size() > (unsigned int)Blob_size){
                blobs.push_back(blob);
            }
            label_count++;
        }
    }
    skeletonLog.printLog("skeleton thread FindBlobs() exit");
}
/***************************************
 * Function〕Get Image
***************************************/
vector<Mat> skeleton::getImg(){
    skeletonLog.printLog(logAddMem("skeleton::getImg()"));
    return pick_Img;
}
/***************************************
 * Function〕Get Rect
***************************************/
vector<Rect> skeleton::getRect(){
    skeletonLog.printLog(logAddMem("skeleton::getRect()"));
    return pick_Rect;
}
/**************************************
 * [Func] : No Detection
**************************************/
bool skeleton::NoDetection(){
    skeletonLog.printLog(logAddMem("skeleton::NoDetection()"));
    return NoDetect;
}
/**************************************
 * [Func] : 判斷手的圖片是否為長方形(垂直放下情況)
 *
**************************************/
bool skeleton::Hand_shape_determinate(Mat oriframe,Point point[9],Mat frame,Rect ROI){
    skeletonLog.printLog(logAddMem("skeleton::Hand_shape_determinate() enter"));
    if(frame.rows / frame.cols > 2 ){
        skeletonLog.printLog(logAddMem("skeleton::Hand_shape_determinate() exit1"));
        return false;
    }
    if((ROI.y+ROI.height) > point[0].y){
        if( point[0].x > ROI.x ){
            if(point[0].x - ROI.width/2 > ROI.x ){
                skeletonLog.printLog(logAddMem("skeleton::Hand_shape_determinate() exit2"));
                return false;
            }
        }
        if(point[0].x < ROI.x){
            if(point[0].x + ROI.width/2 < ROI.x ){
                skeletonLog.printLog(logAddMem("skeleton::Hand_shape_determinate() exit3"));
                return false;
            }
        }
    }
    if(oriframe.rows < frame.rows*3 && oriframe.cols < frame.cols*2){
        skeletonLog.printLog(logAddMem("skeleton::Hand_shape_determinate() exit4"));
        return false;
    }
    //cout<<"oriframe : "<< oriframe.rows<<" , "<<oriframe.cols<<endl;
    //cout<<"frame : "<< frame.rows <<" , "<<frame.cols<<endl;
    skeletonLog.printLog(logAddMem("skeleton::Hand_shape_determinate() exit5"));
    return true;
}

string skeleton::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}

