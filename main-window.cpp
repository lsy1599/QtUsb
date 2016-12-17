#include "main-window.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
{
    _usbContainer = new UsbContainer(&_log);
    ui->setupUi(this);
    QStringList usbLogList = _log.split("\n");
    for(int i=0; i<usbLogList.size(); i++) {
        if(usbLogList.at(i).contains("Error")) {
            ui->usbListWidget->addItem(usbLogList.at(i));
            ui->usbListWidget->item(i)->setTextColor(Qt::red);
        } else {
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
    for(int i=0; i<usbLogList.size(); i++) {
        if(usbLogList.at(i).contains("Error")) {
            ui->usbListWidget->addItem(usbLogList.at(i));
            ui->usbListWidget->item(i)->setTextColor(Qt::red);
        } else {
            ui->usbListWidget->addItem(usbLogList.at(i));
            ui->usbListWidget->item(i)->setTextColor(Qt::green);
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    QStringList usbLogList = _usbContainer->listNonRootDevices();
    ui->usbListWidget->clear();
    for(int i=0; i<usbLogList.size(); i++) {
        ui->usbListWidget->addItem(usbLogList.at(i));
    }
}

void MainWindow::on_pushButton_writeToDev_clicked()
{
    QStringList usbLogList;
    QString string =_usbContainer->writeToDevice(_clickedProduct);
    usbLogList.append("Info: \n" + string);
    ui->usbListWidget->clear();
    ui->usbListWidget->addItem(usbLogList.at(0));
}

void MainWindow::on_usbListWidget_clicked(const QModelIndex &index)
{
    _clickedProduct = (ui->usbListWidget->item(index.row())->text());
    ui->usbListWidget->clear();
    ui->usbListWidget->addItems(_usbContainer->getDeviceInfo(_clickedProduct));
}
