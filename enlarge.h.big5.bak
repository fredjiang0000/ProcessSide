#ifndef ENLARGE_H
#define ENLARGE_H

#include <QWidget>
#include "mainwindow.h"

namespace Ui {
class Enlarge;
}

class MainWindow;

class Enlarge : public QWidget
{
    Q_OBJECT

public:
    explicit Enlarge(MainWindow *main_window,QWidget *parent = 0);
    ~Enlarge();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void image_enlarge(const QImage& img);

private:
    Ui::Enlarge *ui;
    MainWindow *main_window;
};

#endif // ENLARGE_H
