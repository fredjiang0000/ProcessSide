#include "widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QAction>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
: QLabel(parent)
{
  selectionStarted=false;

  QAction *saveAction=contextMenu.addAction("Save");

  connect(saveAction,SIGNAL(triggered()),this,SLOT(saveSlot()));
}

Widget::~Widget()
{

}

void Widget::paintEvent(QPaintEvent *e)
{
     QLabel::paintEvent(e);
     QPainter painter(this);
     painter.setPen(QPen(QBrush(QColor(0,0,0,180)),1,Qt::DashLine));
     painter.setBrush(QBrush(QColor(255,255,255,120)));
     painter.drawRect(selectionRect);
}

void Widget::mousePressEvent(QMouseEvent *e)
{
     if (e->button()==Qt::RightButton)
     {
        if (selectionRect.contains(e->pos()))
            contextMenu.exec(this->mapToGlobal(e->pos()));
     }
     else
     {
        selectionStarted=true;
        selectionRect.setTopLeft(e->pos());
        selectionRect.setBottomRight(e->pos());
     }
}

void Widget::mouseMoveEvent(QMouseEvent *e)
{
    if (selectionStarted)
    {
        selectionRect.setBottomRight(e->pos());
        repaint();
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *e)
{
     selectionStarted=false;
}

void Widget::saveSlot()
{
   QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Save File"),
   "C:\\Users\\andyu_000\\Pictures",
   QObject::tr("Images (*.jpg)"));
   this->pixmap()->copy(selectionRect).save(fileName);
}
