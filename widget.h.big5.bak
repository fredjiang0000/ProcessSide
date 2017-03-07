#ifndef WIDGET
#define WIDGET

#include <QLabel>
#include <QMenu>

class Widget : public QLabel
{
Q_OBJECT

public:
Widget(QWidget *parent = 0);
~Widget();

protected:
void paintEvent(QPaintEvent *e);
void mousePressEvent(QMouseEvent *e);
void mouseMoveEvent(QMouseEvent *e);
void mouseReleaseEvent(QMouseEvent *e);

private:
bool selectionStarted;
QRect selectionRect;
QMenu contextMenu;

private slots:
void saveSlot();

};

#endif // WIDGET

