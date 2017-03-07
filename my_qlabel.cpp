#include "my_qlabel.h"
#include <QtDebug>

//建構子
//繼承QLabel物件,增加Mouse Event
my_qlabel::my_qlabel(QWidget *parent) :
    QLabel(parent)
{
}

//解構子
my_qlabel::~my_qlabel()
{
}

//滑鼠移動事件
void my_qlabel::mouseMoveEvent(QMouseEvent *ev)
{
    this->x = ev->x();
    this->y = ev->y();
    //qDebug() << "X : " << this->x << " Y : " << this->y;
    emit Mouse_Pos(ev);
}

//滑鼠按下事件
void my_qlabel::mousePressEvent(QMouseEvent *ev)
{
    emit Mouse_Pressed(ev);
}
//滑鼠放開事件
void my_qlabel::mouseReleaseEvent(QMouseEvent *ev)
{
    emit Mouse_Released(ev);
}
//滑鼠離開事件
void my_qlabel::leaveEvent(QEvent *)
{
    qDebug() << "Mouse Left !";
    emit Mouse_Left();
}



