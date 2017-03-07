#ifndef MY_QLABEL_H
#define MY_QLABEL_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QEvent>

//繼承QLabel物件,增加Mouse Event
class my_qlabel : public QLabel
{
    Q_OBJECT
public:
    explicit my_qlabel(QWidget *parent = 0);
    ~my_qlabel();


    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void leaveEvent(QEvent *);

    int x,y;
signals:
    void Mouse_Pressed(QMouseEvent *ev);
    void Mouse_Released(QMouseEvent *ev);
    void Mouse_Pos(QMouseEvent *ev);
    void Mouse_Left();


public slots:
};

#endif // MY_QLABEL_H
