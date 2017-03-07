#ifndef CONCURRENTQUEUE
#define CONCURRENTQUEUE

#include <QMutex>
#include <QWaitCondition>
#include <queue>
#include <QtDebug>
#include <iostream>
#include <ctime>
#include "outputLog.h"
using namespace std;


template<typename Data>
class ConcurrentQueue
{
public:
    ConcurrentQueue();
    ~ConcurrentQueue();
    void setClosed(bool state);
    bool getClosed();
    bool empty();
    void push(Data new_data);//一般Push
    bool wait_and_push(Data const& data, const int max_size);//created to allow for a limited queue size
    void push_and_pop(Data new_data, const int max_size);//Push到Buffer滿時, 會pop最舊的一張Frame
    bool try_pop(Data& popped_value);
    bool wait_and_pop(Data& popped_value);   
    int getSizeOfFrameBuffer();//get the number of frames in the buffer
    void WakeAllCondition();//Wake all the waiting condition(重新連線使用到)
private:
    std::queue<Data> the_queue;
    QMutex the_mutex;
    QWaitCondition the_condition_variable;
    bool closed;
    bool doStopPush,doStopPop;
    //將Log訊息加上此物件的記憶體位址
    string logAddMem(const string messages);
    outputLog *myLog;
};

template<typename Data>
ConcurrentQueue<Data>::ConcurrentQueue()
{
    myLog = new outputLog("ConcurrentQueue.log");
    myLog->printLog(logAddMem("ConcurrentQueue<Data>::ConcurrentQueue()"));
}
template<typename Data>
ConcurrentQueue<Data>::~ConcurrentQueue()
{
    myLog->printLog(logAddMem("ConcurrentQueue<Data>::~ConcurrentQueue()"));
    delete this->myLog;
    this->myLog = NULL;
}


template<typename Data>
void ConcurrentQueue<Data>::setClosed(bool state)
{
    QMutexLocker locker(&the_mutex);
    closed = state;
}

template<typename Data>
bool ConcurrentQueue<Data>::getClosed()
{
    QMutexLocker locker(&the_mutex);
    return closed;
}

template<typename Data>
void ConcurrentQueue<Data>::push(Data new_data)
{
    QMutexLocker locker(&the_mutex);
//    qDebug() << "Buffer中的Frame : " << the_queue.size() << "個";
    the_queue.push(new_data);
    the_condition_variable.wakeOne();
}

template<typename Data>
bool ConcurrentQueue<Data>::empty()
{
    QMutexLocker locker(&the_mutex);
    return the_queue.empty();
}

template<typename Data>
bool ConcurrentQueue<Data>::try_pop(Data& popped_value)
{
    QMutexLocker locker(&the_mutex);
    if(the_queue.empty())
    {
//        qDebug() << "Queue Buffer 已經空了!";
        return false;
    }
    popped_value = the_queue.front();
    the_queue.pop();
    the_condition_variable.wakeOne();
    return true;
}

//Push到Buffer滿時, 會pop最舊的一張Frame
template<typename Data>
void ConcurrentQueue<Data>::push_and_pop(Data new_data, const int max_size)
{
    QMutexLocker locker(&the_mutex);

    string log_msg="ConcurrentQueue<Data>::push_and_pop() enter1(Buffer's Size:";
    log_msg+= to_string(the_queue.size()); log_msg+=")";
    myLog->printLog(logAddMem(log_msg));

    if(the_queue.size() >= max_size)
    {
        myLog->printLog(logAddMem("ConcurrentQueue<Data>::push_and_pop() enter2(Pop a frame)"));
        the_queue.pop();
    }
    the_queue.push(new_data);
    the_condition_variable.wakeOne();
    myLog->printLog(logAddMem("ConcurrentQueue<Data>::push_and_pop() exit(Push a frame)"));
}

template<typename Data>
bool ConcurrentQueue<Data>::wait_and_pop(Data& popped_value)
{
    QMutexLocker locker(&the_mutex);

    string log_msg="ConcurrentQueue<Data>::wait_and_pop() enter1(Buffer's Size:";
    log_msg+=to_string(the_queue.size()); log_msg+=")";
    myLog->printLog(logAddMem(log_msg));

    doStopPop = false;
    if(the_queue.empty())
    {
        myLog->printLog(logAddMem("ConcurrentQueue<Data>::wait_and_pop() enter2(Buffer is empty, waiting...)"));
        the_condition_variable.wait(&the_mutex);
        if(doStopPop)
        {
            myLog->printLog(logAddMem("ConcurrentQueue<Data>::wait_and_pop() enter3(doStopPop)"));
            return false;
        }
        myLog->printLog(logAddMem("ConcurrentQueue<Data>::wait_and_pop() enter4(Be waked)"));
    }
    popped_value = the_queue.front();
    the_queue.pop();
    myLog->printLog(logAddMem("ConcurrentQueue<Data>::wait_and_pop() exit(Pop a frame)"));
    return true;
}

//created to allow for a limited queue size
template<typename Data>
bool ConcurrentQueue<Data>::wait_and_push(Data const& data, const int max_size)
{
    QMutexLocker locker(&the_mutex);
    doStopPush = false;
    cout << "Buffer Frame Size:" << the_queue.size() <<endl;
    if(the_queue.size() >= max_size)
    {
        cout << "Queue Buffer reached Max_Size" <<endl;
        the_condition_variable.wait(&the_mutex);
        if(doStopPush)
        {
            cout << "Break the Push Waiting State." <<endl;
            return false;
        }
    }
    the_queue.push(data);
    the_condition_variable.wakeOne();
    return true;
}


template<typename Data>
int ConcurrentQueue<Data>::getSizeOfFrameBuffer()
{
    QMutexLocker locker(&the_mutex);
    return the_queue.size();
}

template<typename Data>
void ConcurrentQueue<Data>::WakeAllCondition()
{
    QMutexLocker locker(&the_mutex);
    myLog->printLog(logAddMem("ConcurrentQueue<Data>::WakeAllCondition() enter"));
    doStopPush = true;
    doStopPop = true;
    the_condition_variable.wakeAll();
    myLog->printLog(logAddMem("ConcurrentQueue<Data>::WakeAllCondition() exit"));
}

template<typename Data>
string ConcurrentQueue<Data>::logAddMem(const string messages)
{
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    std::string log_msg = messages; log_msg+="("; log_msg+=ss.str(); log_msg+=")";
    return log_msg;
}


#endif // CONCURRENTQUEUE

