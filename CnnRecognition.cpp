#include "stdafx.h"
#include "CnnRecognition.h"
#include <QtNetwork>
#include <stdio.h>

bool CnnRecognition::cellDetect(Mat img){

    vector<uchar> buff;//buffer for coding
    vector<int> param = vector<int>(2);
    param[0]=CV_IMWRITE_JPEG_QUALITY;
    param[1]=100;//default(95) 0-100

    imencode(".jpg",img,buff,param);

    //得到要傳輸的frmae大小
    int sz = buff.size();

    //給Python 知道等下要傳送多大的圖片
    sock_to_python->write((const char*)&sz,4);
    sock_to_python->flush();

    //傳送圖片
    sock_to_python->write((const char*)&buff[0],sz);
    sock_to_python->flush();

    char result;
    QByteArray recv_data;

    //清空
    recv_data.clear();

    //一職block到 socket的buffer有足夠的資料 (1 byte)
    //while(sock_to_python->bytesAvailable() < 1){
    //}

    //接收python回傳的結果,此結果是一字元, '0' (代表預測有手機) or '1' (代表預測無手機)
    recv_data = sock_to_python->read(1);
    printf("RECV STR: %s\n",recv_data.data());


    result  = recv_data.data()[0];
    //result = ReverseInt(result);

    printf("RESULT: %c , int: %d\n",result,result);
    fflush(stdout);

    if(result == '0')
        return true;

    return false;
}

CnnRecognition::CnnRecognition(){
}

CnnRecognition::~CnnRecognition(){

}

// 無用 , 用來轉換big endian和little endian
int CnnRecognition::ReverseInt( const float Num )
{
   int retVal;
   char *intToConvert = ( char* ) & Num;
   char *returnInt = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnInt[0] = intToConvert[3];
   returnInt[1] = intToConvert[2];
   returnInt[2] = intToConvert[1];
   returnInt[3] = intToConvert[0];

   return retVal;
}
