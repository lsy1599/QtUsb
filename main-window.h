#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "usb-container.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_usbReloadPushButton_clicked();

    void on_pushButton_clicked();

    void on_pushButton_writeToDev_clicked();

    void on_usbListWidget_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    QString _log;
    UsbContainer* _usbContainer;
    int _valid_row;
};

#endif // MAINWINDOW_H
