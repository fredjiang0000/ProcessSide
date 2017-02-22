#ifndef OUTPUTLOG_H
#define OUTPUTLOG_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <QMutex>
using namespace std;



class outputLog
{
public:
    outputLog(const string filename);
    ~outputLog();
    //印出Log訊息
    void printLog(const string messages);
private:
    //Get Current Time
    string get_current_time();
    //Get Current Time,紀錄Log時的時間戳記
    ostream& get_current_time(ostream &output);
    //Get Current Date,用來讓Log檔名依照日期
    string get_current_date();
    //檔名
    string fileName,tempFileName;
    //檔案中的訊息行數
    int logLines;
    bool writeInTemp;
    QMutex the_mutex;
};

#endif // OUTPUTLOG_H

