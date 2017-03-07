#include "objtrack.h"
#include "bgsubmog.h"
#include <map>
#include <math.h>

//outputLog myLog("objTrack.log");
/******************** Constructor ***************************/
/*******************************************
objTrack Constructor : human width height ratio,
minimun object size to filter
= Initial BackgroundSubtractorMOG, setSVMDetector(for HOG),
tracking state, whRatio, minObjSize
********************************************/
objTrack::objTrack(outputLog *mylog):tracking(false),minObjSize(0),whRatio(1/3)
{
    cout << "objTrack::objTrack(1)" << endl;
    this->myLog = mylog;
    this->myLog->printLog(logAddMem("objTrack::objTrack(1) enter"));
    BackgroundSubtractorMOG mog;
    bgProcessor = bgSubMog(mog,this->myLog);
    this->myLog->printLog(logAddMem("objTrack::objTrack(1) exit"));
}

objTrack::objTrack(float whR,outputLog *mylog):tracking(false),minObjSize(0),whRatio(whR)
{
    this->myLog = mylog;
    this->myLog->printLog(logAddMem("objTrack::objTrack(2) enter"));
    BackgroundSubtractorMOG mog;
    bgProcessor = bgSubMog(mog,this->myLog);
    this->myLog->printLog(logAddMem("objTrack::objTrack(2) exit"));
}

objTrack::objTrack(int minSize,outputLog *mylog):tracking(false),minObjSize(minSize),whRatio(1)
{
    this->myLog = mylog;
    this->myLog->printLog(logAddMem("objTrack::objTrack(3) enter"));
    BackgroundSubtractorMOG mog;
    bgProcessor = bgSubMog(mog,this->myLog);
    this->myLog->printLog(logAddMem("objTrack::objTrack(3) exit"));
}

objTrack::objTrack(float whR, int minSize,outputLog *mylog):tracking(false),minObjSize(minSize),whRatio(whR)
{
    this->myLog = mylog;
    this->myLog->printLog(logAddMem("objTrack::objTrack(4) enter"));
    BackgroundSubtractorMOG mog;
    bgProcessor = bgSubMog(mog,this->myLog);
    this->myLog->printLog(logAddMem("objTrack::objTrack(4) exit"));
}


objTrack::~objTrack(){
    this->myLog->printLog(logAddMem("objTrack::~objTrack()"));
}

/******************** Function ***************************/
/*******************************************
getObj :
= return tracking object except group object
********************************************/
vector<Rect> objTrack::getObj(){
    this->myLog->printLog(logAddMem("objTrack::getObj() enter"));
    vector<Rect> obj;

    for(int i = 0 ; i < objList.size() ; ++i){

        if(!objList.at(i).isGroup()){
            Rect newObj;
            newObj.x = objList.at(i).getObjRoi().x*2;
            newObj.y = objList.at(i).getObjRoi().y*2;
            newObj.width = objList.at(i).getObjRoi().width*2;
            newObj.height = objList.at(i).getObjRoi().height*2;
            obj.push_back(newObj);

            //            obj.push_back(objList.at(i).getObjRoi());
        }
    }
    this->myLog->printLog(logAddMem("objTrack::getObj() exit"));
    return obj;
}
/*******************************************
getGroup :
= return group object except normal object
********************************************/
vector<Rect> objTrack::getGroup(){
    this->myLog->printLog(logAddMem("objTrack::getGroup() enter"));
    vector<Rect> group;

    for(int i = 0 ; i < objList.size() ; ++i){
        if(objList.at(i).isGroup()){
            Rect newObj;
            newObj.x = objList.at(i).getObjRoi().x*2;
            newObj.y = objList.at(i).getObjRoi().y*2;
            newObj.width = objList.at(i).getObjRoi().width*2;
            newObj.height = objList.at(i).getObjRoi().height*2;
            group.push_back(newObj);

            //            group.push_back(objList.at(i).getObjRoi());
        }
    }
    this->myLog->printLog(logAddMem("objTrack::getGroup() exit"));
    return group;
}

/*******************************************
tracker : frame
= combine background subtraction and object classification
+ change tracking state and initial minObjSize
+ call bgProcessor to get object with current frame
+ deal with object for current frame and last frame to classification
********************************************/
bool objTrack::tracker(Mat frame){

    string log_msg = "objTrack::tracker() enter1(frame.cols:";
    log_msg+= to_string(frame.cols); log_msg+=" frame.rows:";
    log_msg+= to_string(frame.rows); log_msg+=")";
    this->myLog->printLog(logAddMem(log_msg));
    Mat smallframe;
    if(frame.cols)
    resize(frame,smallframe,Size(frame.cols/2,frame.rows/2));

    if(!tracking){
        tracking = true;
        if(minObjSize == 0)
            minObjSize = smallframe.cols*smallframe.rows/100 ;
    }
    this->myLog->printLog(logAddMem("objTrack::tracker() enter2"));
    vector<Rect> mogROI = bgProcessor.objTrack(smallframe,minObjSize);


    //check frame error
    bool nowFrameErrFlag;
    for(int i = 0; i < mogROI.size() ; ++i){
        if(mogROI.at(i).width > smallframe.cols*(2/(float)3) && mogROI.at(i).height > smallframe.rows*(2/(float)3)){
            nowFrameErrFlag = true;
        }else{
            nowFrameErrFlag = false;
        }
    }
    this->myLog->printLog(logAddMem("objTrack::tracker() enter3"));

    if(FrameErrFlag && nowFrameErrFlag){
        this->myLog->printLog(logAddMem("objTrack::tracker() enter4"));
        //frame ERROR continue;
        errTime--;
        if(errTime == 0){
            objList.clear();
        }else if(errTime < 0){ //prevent infinity error
            errTime == 0;
        }
    }else if(nowFrameErrFlag){
        this->myLog->printLog(logAddMem("objTrack::tracker() enter5"));
        //frame ERROR first time;
        FrameErrFlag = nowFrameErrFlag;
        errTime--;
    }else {
        this->myLog->printLog(logAddMem("objTrack::tracker() enter6"));
        //frame correct;
        if(FrameErrFlag){
            this->myLog->printLog(logAddMem("objTrack::tracker() enter6-1"));
            FrameErrFlag = nowFrameErrFlag;
            errTime = 3;
        }

        vector<int> delSort;
        if(objList.empty() && mogROI.size() == 0){
            this->myLog->printLog(logAddMem("objTrack::tracker() enter6-2"));
            //empty enviroment

        }
        else if(objList.empty() && mogROI.size() > 0){
            this->myLog->printLog(logAddMem("objTrack::tracker() enter6-3"));
            //start for tracking
            for(int i = 0; i < mogROI.size(); ++i){
                objData newObj;
                newObj.setObjRoi(mogROI.at(i));

                objList.push_back(newObj);
            }
        }
        else{
            this->myLog->printLog(logAddMem("objTrack::tracker() enter6-4"));
            //check obj move & create Mapping table
            multimap<int,int> correspond;           //mogROI:objList
            multimap<int,int> inverseCorrespond;    //objList:mogROI
            for(int i = 0 ; i < mogROI.size() ; ++i){
                for(int j = 0 ; j < objList.size() ; ++j){
                    Rect overlap = mogROI.at(i)&objList.at(j).getObjRoi();
                    if(overlap.width*overlap.height > 100){ //have overlap
                        correspond.insert(make_pair(i,j));
                        inverseCorrespond.insert(make_pair(j,i));
                    }
                }
            }

            //Deal with [ mogROI , objList ] situation
            //Round Current Object. deal with overlap, new, collide
            for(int i = 0 ; i < mogROI.size() ; ++i){
                int beforeOBJ = correspond.find(i)->second;
                int afterMOG = i;

                if(correspond.count(afterMOG) == 1 && inverseCorrespond.count(beforeOBJ) == 1){
                    //same obj with after & before
                    //[1:1]
                    /*     reduce big change   */
                    Point centroidOld,centroidNew,startOld,startNew,endOld,endNew;
                    centroidOld.x = objList.at(beforeOBJ).getObjRoi().x + objList.at(beforeOBJ).getObjRoi().width/2;
                    centroidOld.y = objList.at(beforeOBJ).getObjRoi().y + objList.at(beforeOBJ).getObjRoi().height/2;
                    centroidNew.x = mogROI.at(i).x + mogROI.at(i).width/2;
                    centroidNew.y = mogROI.at(i).y + mogROI.at(i).height/2;
                    startOld.x = objList.at(beforeOBJ).getObjRoi().x ;
                    startOld.y = objList.at(beforeOBJ).getObjRoi().y ;
                    startNew.x = mogROI.at(i).x ;
                    startNew.y = mogROI.at(i).y ;
                    double cenDis = cv::norm( centroidOld - centroidNew);
                    double startDis = cv::norm(startOld - startNew);
                    Rect roiOverlap = objList.at(beforeOBJ).getObjRoi()&mogROI.at(i);
                    if(abs(startDis) > mogROI.at(i).width && abs(cenDis) > mogROI.at(i).width*2){
                        objData newObj;
                        newObj.setObjRoi(mogROI.at(i));
                        newObj.setNew(true);
                        newObj.setGroup(false);
                        objList.push_back(newObj);
                    }/*    reduce big change   */
                    else if((mogROI.at(i).x - roiOverlap.x) < 20 &&
                            (mogROI.at(i).y - roiOverlap.y) < 20 &&
                            (mogROI.at(i).width - roiOverlap.width) < 20 &&
                            (mogROI.at(i).height - roiOverlap.height) < 20){
                        /*    background update   */
                        Rect roiOr = objList.at(beforeOBJ).getObjRoi()|mogROI.at(i);
                        objList.at(beforeOBJ).setObjRoi(roiOr);
                    }
                    else{
                        objList.at(beforeOBJ).setObjRoi(mogROI.at(i));
                    }
                    objList.at(beforeOBJ).setObjRoi(mogROI.at(i));
                    if(objList.at(beforeOBJ).isNew()){
                        objList.at(beforeOBJ).setNew(false);
                    }

                }else if(correspond.count(afterMOG) > 1){
                    // collide with each second
                    //[1:N]
                    multimap<int,int>::iterator it;
                    //                cout << "collide :: " << correspond.count(afterMOG) << endl;
                    for(it = correspond.equal_range(afterMOG).first;it != correspond.equal_range(afterMOG).second ; ++it ){
                        delSort.push_back((*it).second);
                    }

                    objData newObj;
                    newObj.setObjRoi(mogROI.at(afterMOG));
                    newObj.setNew(true);
                    if(mogROI.at(afterMOG).height > mogROI.at(afterMOG).width*2){
                        newObj.setGroup(false);
                    }else{
                        newObj.setGroup(true);
                    }
                    objList.push_back(newObj);
                }else if(correspond.count(afterMOG) == 0){
                    // new obj
                    //[0:]
                    //                cout << "new obj" << endl;
                    objData newObj;
                    newObj.setObjRoi(mogROI.at(afterMOG));
                    newObj.setNew(true);
                    objList.push_back(newObj);
                }
            }
            this->myLog->printLog(logAddMem("objTrack::tracker() enter6-5"));
            //Round Last Object
            for(int i = 0 ; i < objList.size() ; ++i){
                int beforeOBJ = i;
                int afterMOG = inverseCorrespond.find(i)->second;

                if(inverseCorrespond.count(beforeOBJ) > 1){
                    // separate with each obj
                    //[N:1]
                    //                cout << "separate" << endl;
                    delSort.push_back(beforeOBJ);
                    //                objList.erase(objList.begin() + beforeOBJ);
                    multimap<int,int>::iterator it;
                    for(it = inverseCorrespond.equal_range(beforeOBJ).first;it != inverseCorrespond.equal_range(beforeOBJ).second ; ++it ){
                        objData newObj;
                        newObj.setObjRoi(mogROI.at((*it).second));
                        newObj.setNew(true);
                        //                        if((mogROI.at((*it).second).height/mogROI.at((*it).second).width) < 3 ){
                        newObj.setGroup(false);
                        //                        }
                        objList.push_back(newObj);
                    }
                }else if(inverseCorrespond.count(beforeOBJ) == 0){
                    // obj disapear or stay
                    //[:0]
                    if((objList.at(beforeOBJ).getObjRoi().x < smallframe.cols/50 ||
                        objList.at(beforeOBJ).getObjRoi().y < smallframe.rows/50 ||
                        objList.at(beforeOBJ).getObjRoi().x + objList.at(beforeOBJ).getObjRoi().width > smallframe.cols - smallframe.cols/50 ||
                        objList.at(beforeOBJ).getObjRoi().y + objList.at(beforeOBJ).getObjRoi().height > smallframe.rows - smallframe.rows/50)
                            && !objList.at(beforeOBJ).isNew())
                    {   // near edge : obj disapear
                        delSort.push_back(beforeOBJ);
                        //                    objList.erase(objList.begin() + beforeOBJ);
                        //                    cout << "disapear" << endl;
                    }
                }
            }

        }
        this->myLog->printLog(logAddMem("objTrack::tracker() enter6-6"));
        //delete obj
        sort(delSort.begin(),delSort.end());
        reverse(delSort.begin(),delSort.end());
        for(int i = 0; i < delSort.size() ; ++i ){
            objList.erase(objList.begin() + delSort.at(i));
        }
        this->myLog->printLog(logAddMem("objTrack::tracker() enter6-7"));

    }
    this->myLog->printLog(logAddMem("objTrack::tracker() exit"));
    return false;
}

/*******************************************
binaryImage :
= return forground mask
********************************************/
Mat objTrack::binaryImage(){
    this->myLog->printLog(logAddMem("objTrack::binaryImage() enter"));
    Mat i = bgProcessor.getMask();
    this->myLog->printLog(logAddMem("objTrack::binaryImage() exit"));
    return i;
}

/*******************************************
Get_HumanNum : object rectangle
= return object number
********************************************/
int objTrack::Get_HumanNum(Rect obj){
    this->myLog->printLog(logAddMem("objTrack::Get_HumanNum() enter"));
    int Human_num = 0;
    for(int i=0;i<(int)objList.size();i++){
        if(obj == objList.at(i).getObjRoi() ){
            Human_num = i;
        }
    }
    this->myLog->printLog(logAddMem("objTrack::Get_HumanNum() exit"));
    return Human_num;
}

string objTrack::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}


