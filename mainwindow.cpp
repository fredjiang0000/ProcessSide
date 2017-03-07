#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>

//建構子 程式最主要的Window視窗
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    F_enlarge = new Enlarge(this);//放大檢視視窗
    F_connection = new connection(this);//連線設定視窗


    //放大檢視的傳遞參數,參數:影像
    QObject::connect(this,SIGNAL(enlarge_update(const QImage&)),
                     F_enlarge,SLOT(image_enlarge(const QImage&)));


    open_enlarge = false;
}

//解構子
MainWindow::~MainWindow()
{
    cout << "MainWindow::~MainWindow()" << endl;
    delete ui;
}
//將放大檢視Flag設為False
void MainWindow::setOpenEnlarge(bool flag)
{
    this->open_enlarge = flag;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->F_connection){
        this->F_connection->close();
        delete this->F_connection;
    }
    if(this->F_enlarge){
        this->F_enlarge->close();
        delete this->F_enlarge;
    }
}

//===================連線設定======================
void MainWindow::on_pushButton_connection_clicked()
{  
    F_connection->show();
}
//===================放大檢視======================
void MainWindow::on_pushButton_enlarge_clicked()
{
        if(F_connection->isCamDisconnection())
        {
            QMessageBox::warning(this,tr("警告"),tr("此監視器畫面尚未連線!"));
            return;
        }
        this->open_enlarge = true;
        F_enlarge->show();
}

//[Slot]============更新影像畫面==================
void MainWindow::updatePlayerUI(const QImage img)
{
        //若接收到的QImage不為空
        if (!img.isNull()){
            if(this->open_enlarge==true)//放大檢視
            {
                emit enlarge_update(img);
            }
            ui->label_1->setPixmap(QPixmap::fromImage(img).scaled(ui->label_1->size(),
                                                                  Qt::KeepAspectRatio, Qt::FastTransformation));
        }
        else{
            ui->label_1->setStyleSheet("Background-color:#000");//顯示黑色畫面
            ui->label_1->update();
            cout << "updatePlayerUI : Image is NULL";
        }
}
//===============更新在視窗的Log訊息========================
//參數: (1)msg為Log訊息內容 (2)level為Log訊息的顏色(0:綠,1:藍,2:紅)
void MainWindow::UpdateMyLog(QString msg,const int level)
{
    QTextCursor cursor = ui->LogEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->LogEdit->setTextCursor(cursor);

    QString alertHtml = "<font color=\"Red\">";
    QString notifyHtml = "<font color=\"#009900\">";
    QString infoHtml = "<font color=\"Blue\">";
    QString endHtml = "</font><br>";
    switch(level)
    {
        case 0: msg = notifyHtml % msg; break;//綠色字
        case 1: msg = infoHtml % msg; break;//藍色字
        case 2: msg = alertHtml % msg; break;//紅色字
        default: msg = infoHtml % msg; break;
    }
    msg = msg % endHtml;
    msg = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "  " + msg;//加上時間戳記
    ui->LogEdit->insertHtml(msg);
}
