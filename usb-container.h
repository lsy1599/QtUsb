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
    QString writeToDevice(QString &productString);
    QStringList getDeviceInfo(int i);
    QStringList getDeviceInfo(QString &stringi);
};

#endif // USBCONTAINER_H
