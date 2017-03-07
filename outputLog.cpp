#include "outputLog.h"
#include <string>

outputLog::outputLog(const string filename)
{
    this->fileName = filename;
    int i = filename.find('.');
    string tempName = filename.substr(0,i);
    tempName += "_temp.log";
    this->tempFileName = tempName;
    this->logLines = 0;
    this->writeInTemp = false;
}

outputLog::~outputLog()
{

}

string outputLog::get_current_time()
{
    struct tm newtime;
    __time64_t long_time;
    errno_t err;
    // Get time as 64-bit integer.
    _time64( &long_time );
    // Convert to local time.
    err = _localtime64_s( &newtime, &long_time );
    if (err)
    {
        printf("Invalid argument to _localtime64_s.");
        exit(1);
    }
    string current_time;
    current_time  = to_string(newtime.tm_year + 1900);
    current_time += "/"; current_time += to_string(newtime.tm_mon + 1);
    current_time += "/"; current_time += to_string(newtime.tm_mday);
    current_time += " "; current_time +=to_string(newtime.tm_hour);
    current_time += ":"; current_time +=to_string(newtime.tm_min);
    current_time += ":"; current_time +=to_string(newtime.tm_sec);
    return current_time;
}

ostream &outputLog::get_current_time(ostream &output)
{
    struct tm newtime;
    __time64_t long_time;
    errno_t err;
    // Get time as 64-bit integer.
    _time64( &long_time );
    // Convert to local time.
    err = _localtime64_s( &newtime, &long_time );
    if (err)
    {
        printf("Invalid argument to _localtime64_s.");
        exit(1);
    }
    output  << (newtime.tm_year + 1900) << '/'
            << (newtime.tm_mon + 1) << '/'
            <<  newtime.tm_mday << " "
             <<  newtime.tm_hour << ":" << newtime.tm_min << ":" << newtime.tm_sec;
    return output;
}
string outputLog::get_current_date()
{
    struct tm newtime;
    __time64_t long_time;
    errno_t err;
    // Get time as 64-bit integer.
    _time64( &long_time );
    // Convert to local time.
    err = _localtime64_s( &newtime, &long_time );
    if (err)
    {
        printf("Invalid argument to _localtime64_s.");
        exit(1);
    }
    string current_date;
    current_date  = to_string(newtime.tm_year + 1900);
    current_date += "-"; current_date += to_string(newtime.tm_mon + 1);
    current_date += "-"; current_date += to_string(newtime.tm_mday);
    return current_date;
}

//在Log資料夾中印出Log訊息
//【Log檔案只保留最新訊息的方法】
//Step1:當Log訊息量超過threshold, 則寫到另一個temp檔
//Step2:temp檔再超過threshold, 則刪除上一個原Log檔, 將temp檔命名為Log檔名
//Step3:開啟一個新的temp檔, 繼續將訊息寫在temp檔, 重複Step2~Step3動作
void outputLog::printLog(const string messages)
{
    QMutexLocker locker(&the_mutex);
    ofstream mylog;
    string logFilePath = "./Log/";
    if(!this->writeInTemp)
        logFilePath += this->fileName;
    else
        logFilePath += this->tempFileName;
//    cout << "logFilePath:" << logFilePath << endl;
    mylog.open(logFilePath,ios::out | ios::app);
    if(!mylog)
    {
        cout << get_current_time() << " [Error] Fail to open logFile:"<< this->fileName << endl;
        mylog.close();
//        exit(1);
    }
    else
    {
        get_current_time(mylog);
        mylog << "  " << messages << endl;
        this->logLines++;
        if(this->logLines>20000)
        {
            if(this->writeInTemp==false){
                this->logLines = 0;
                this->writeInTemp = true;
            }
            else{
                string file1Path = "./Log/"; file1Path+=this->fileName;
                string file2Path = "./Log/"; file2Path+=this->tempFileName;
                const char * file1 = file1Path.c_str();
                const char * file2 = file2Path.c_str();
                mylog.close();//檔案要關閉後才能做刪除、修改
                remove(file1);
                rename(file2,file1);
                this->logLines = 0;
//                cout << logFilePath << " Update" << endl;
            }
        }
        mylog.close();
    }
}




