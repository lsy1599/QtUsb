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
    int writeToDevice(QString &productString);
    // TODO
    // Do something to write to exact device...
    // best would be to take String name from device
    // compare to every one device on list
    // return the device
    // but there is one problem - how to grab String from QListWidget
    QStringList getDeviceInfo(int i){
        QString tmp;
        QTextStream(&tmp) << _usbDevices.size();
        QStringList tmpList;
//        tmpList.append(QString(i));
        tmpList.append(tmp);
        QTextStream(&tmp) << i;
        tmpList.append(tmp);
        return tmpList;
//( _usbDevices[i].devInfo() );
//        UsbDev tmpDev;
//        QStringList tmpList;
//        for(int i=0;i<_usbDevices.size();i++)
//        {
//            if(_usbDevices.at(i)._nonSUdoDev == true){
//                tmpDev = _usbDevices.at(i);
//            }
//        }
//        return tmpList;// = tmpDev.devInfo();
    }

    QStringList getDeviceInfo(QString &stringi){
        QStringList tmp;
        size_t size = _usbDevices.size(), i;
        for(i=0; i<size ; ++i){
            if(_usbDevices[i].getProductString() == stringi ){
                break;
            }
        }
        if(i == size){
            tmp.append("Shit no such device");
        }else{
            tmp = _usbDevices[i].devInfo();
        }

        return tmp;
    }
};

#endif // USBCONTAINER_H
