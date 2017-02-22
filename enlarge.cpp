#include "enlarge.h"
#include "ui_enlarge.h"

//建構子
Enlarge::Enlarge(MainWindow *main_window,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Enlarge)
{
    ui->setupUi(this);
    this->main_window = main_window;
}

//解構子
Enlarge::~Enlarge()
{
    delete ui;
}

//Update Image On the Label
void Enlarge::image_enlarge(const QImage& img)//update_image
{
    if (!img.isNull())
    {
        ui->label->setPixmap(QPixmap::fromImage(img).scaled(ui->label->size(),
                                           Qt::KeepAspectRatio, Qt::FastTransformation));
    }
}

//Close Event,將Flag更改
void Enlarge::closeEvent(QCloseEvent *event)
{
     this->main_window->setOpenEnlarge(false);
}
