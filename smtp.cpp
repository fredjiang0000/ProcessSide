#include "smtp.h"
#include <ctime>
#include <QProcess>
#include <iostream>
#include <fstream>
#include "outputLog.h"
using namespace std;

outputLog smtpLog("SMTP.log");

Smtp::Smtp(const string &sender, const string &paswd, const string &host,
           const string &rcpt, const Mat& send_frame, const string event_type,
           const string ocurrTime)
{    
    smtpLog.printLog(logAddMem("Smtp::Smtp() enter"));
    this->sender = sender;
    this->paswd = paswd;
    this->host = host;
    this->rcpt = rcpt;
    send_frame.copyTo(this->send_frame);
    this->event_type = event_type;
    this->occurTime = ocurrTime;
    smtpLog.printLog(logAddMem("Smtp::Smtp() exit"));
}
Smtp::~Smtp(){
    smtpLog.printLog(logAddMem("Smtp::~Smtp()"));
}

//[SLOT] 執行Send Mail
void Smtp::doSendMail()
{
     smtpLog.printLog(logAddMem("Smtp::doSendMail() enter1"));
     qDebug () << "=========== doSendMail() ==========";
    // ===== 主旨 =====
    string subject;
    qDebug () << "event_type:" << QString::fromStdString(event_type);
    if(this->event_type.compare("UsePhone")==0)
        subject = "[Detect Event] Using a cell phone or camera";
    else if(this->event_type.compare("ROI")==0)
        subject = "[Detect Event] There are changes in the designated area";
    // ===== 內文 =====
    string context;
    if(this->event_type.compare("UsePhone")==0)
        context = "Time of Occurrence : " + this->occurTime;
    else if(this->event_type.compare("ROI")==0)
        context = "Time of Occurrence : " + this->occurTime;
    // ===== 附檔 =====
    string file_name = this->event_type;file_name+=".jpg";
    imwrite(file_name, this->send_frame);
    smtpLog.printLog(logAddMem("Smtp::doSendMail() enter2"));

    string userId = cutID(this->sender);//切出ID

    // ===== 參數寫入Bat檔 =====
    ofstream sendMailBat;
    sendMailBat.open("sendMailBat.bat",ios::out);
    if(!sendMailBat){
        cout<<"Fail to open file: sendMailBat" <<endl;
        smtpLog.printLog(logAddMem("Smtp::doSendMail() exit1(Fail to open Bat file)"));
        emit workFinished();
        return;
    }
    sendMailBat << "java -cp javax.mail.jar;. sendmail " <<
                   '"' << userId << '"' << " " <<
                   '"' <<  this->paswd << '"' << " " <<
                   '"' << this->rcpt << '"' << " " <<
                   '"' << this->sender << '"' << " " <<
                   '"' << this->host << '"' << " " <<
                   '"' << file_name << '"' << " " <<
                   '"' << subject << '"' << " " <<
                   '"' << context << '"';
    sendMailBat.close();

    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "sendMailBat.bat");
    if (p.waitForStarted())
    {
       p.waitForFinished();
       qDebug() << p.readAllStandardOutput();
    }
    else{
       qDebug() << "Failed to start";
       smtpLog.printLog(logAddMem("Smtp::doSendMail() exit2(Fail to start QProcess)"));
       emit workFinished();
       return;
    }

    qDebug() << "===== doSendMail() Finished =====";
    smtpLog.printLog(logAddMem("Smtp::doSendMail() exit3(Succeed)"));
    emit workFinished();
}

string Smtp::cutID(string email)
{
    smtpLog.printLog(logAddMem("Smtp::cutID() enter"));
    QString cutMail = QString::fromStdString(email);
    int end = cutMail.indexOf("@");
    cutMail = cutMail.mid(0,end);
    smtpLog.printLog(logAddMem("Smtp::cutID() exit"));
    return cutMail.toUtf8().constData();
}

string Smtp::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}
