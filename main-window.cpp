#include "main-window.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _valid_row(0xFF)                                        // well .. noone shall have more than 255 devs on his pc...
{
    _usbContainer = new UsbContainer(&_log);
    ui->setupUi(this);
    QStringList usbLogList = _log.split("\n");
    for(int i=0;i<usbLogList.size();i++){
        if(usbLogList.at(i).contains("Error")){
            ui->usbListWidget->addItem(usbLogList.at(i));
            ui->usbListWidget->item(i)->setTextColor(Qt::red);
        }else{
            ui->usbListWidget->addItem(usbLogList.at(i));
            ui->usbListWidget->item(i)->setTextColor(Qt::green);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _usbContainer;
}

void MainWindow::on_usbReloadPushButton_clicked()
{
    delete _usbContainer;
    _log.clear();
    _usbContainer = new UsbContainer(&_log);
    QStringList usbLogList = _log.split("\n");
    ui->usbListWidget->clear();
    for(int i=0;i<usbLogList.size();i++){
        if(usbLogList.at(i).contains("Error")){
            ui->usbListWidget->addItem(usbLogList.at(i));
            ui->usbListWidget->item(i)->setTextColor(Qt::red);
        }else{
            ui->usbListWidget->addItem(usbLogList.at(i));
            ui->usbListWidget->item(i)->setTextColor(Qt::green);
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    QStringList usbLogList = _usbContainer->listNonRootDevices();
    ui->usbListWidget->clear();
    for(int i=0;i<usbLogList.size();i++){
        ui->usbListWidget->addItem(usbLogList.at(i));
    }
}

void MainWindow::on_pushButton_writeToDev_clicked()
{
    QStringList usbLogList;
    if(_valid_row!=0xFF){
        QString string;
        _usbContainer->writeToDevice(_valid_row,&string);
        usbLogList.append("Data to be send! \n" + string);
        ui->usbListWidget->clear();
        ui->usbListWidget->addItem(usbLogList.at(0));
        ui->usbListWidget->item(0)->setTextColor(Qt::green);
    }else{
        usbLogList.append("Damn failure");
        ui->usbListWidget->clear();
        ui->usbListWidget->addItem(usbLogList.at(0));
        ui->usbListWidget->item(0)->setTextColor(Qt::red);
    }
}

void MainWindow::on_usbListWidget_clicked(const QModelIndex &index)
{
    ui->usbListWidget->clear();
//    ui->usbListWidget->addItems(_usbContainer->getDeviceInfo(index.row()));
}
