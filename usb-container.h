#ifndef USBCONTAINER_H
#define USBCONTAINER_H
#include "usbdev.h"
#include <QVector>
#include <QTextStream>

class UsbContainer{
private:
    QVector<UsbDev> _usbDevices;
    int _error;
    uint _numOfDev;
    int usbLibInit();
    QString *_errorLog;
public:
    UsbContainer(QString *errorLog);
    ~UsbContainer();
    libusb_device   **_device_list;
    libusb_context  *_ctx;
    QStringList listNonRootDevices();
    int writeToDevice(uint i, QString *ohShiet=0);
    QStringList getDeviceInfo(int i){
        QStringList tmpList;
//        UsbDev tmpDev;
//        QStringList tmpList;
//        for(int i=0;i<_usbDevices.size();i++)
//        {
//            if(_usbDevices.at(i)._nonSUdoDev == true){
//                tmpDev = _usbDevices.at(i);
//            }
//        }
        return tmpList;// = tmpDev.devInfo();
    }
};

#endif // USBCONTAINER_H
