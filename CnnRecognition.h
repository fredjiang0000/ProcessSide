#ifndef CNNRECOGNITION_H
#define CNNRECOGNITION_H

#include "stdafx.h"
#include <QtNetwork>

using namespace std;
using namespace cv;

extern QTcpSocket *sock_to_python;

class CnnRecognition
{
public:
    CnnRecognition();
    ~CnnRecognition();
    bool cellDetect(Mat img);

private:
    int ReverseInt( const float Num );
};

#endif // CNNRECOGNITION_H
