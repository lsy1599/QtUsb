#include "usb-container.h"

namespace {
    static UsbDev deft;
}

UsbContainer::UsbContainer(QString *errorLog) : _error(0), _numOfDev(0), _errorLog(errorLog)
{
    _error = usbLibInit();
    if(_error < 0) QTextStream(_errorLog) << "Error: Initializing libusb" << endl;
    else {
        _numOfDev = libusb_get_device_list(_ctx, &_device_list);
        if (_numOfDev < 1) {
            QTextStream(_errorLog) << "Error: No dev found" << endl;
            return;
        } else {
            for (uint i=0; i<_numOfDev; i++) {
                _usbDevices.push_back(UsbDev(_device_list[i],i,errorLog));
            }
        }
    }
}

UsbContainer::~UsbContainer()
{
    libusb_exit(_ctx);
}

int UsbContainer::usbLibInit()
{
    uint error;
    return error = libusb_init(&_ctx);
}


// Lists all devices which are accesible with no root privileges
QStringList UsbContainer::listNonRootDevices()
{
    QStringList tmpList;
    for(int i=0; i<_usbDevices.size(); i++) {
        if(_usbDevices.at(i).isNonSudoDev()) {
            tmpList.append(_usbDevices.at(i).getProductString());
        }
    }
    return tmpList;
}

QStringList UsbContainer::getDeviceInfo(int i)
{
    QString tmp;
    QTextStream(&tmp) << _usbDevices.size();
    QStringList tmpList;
    tmpList.append(tmp);
    QTextStream(&tmp) << i;
    tmpList.append(tmp);
    return tmpList;
}

QStringList UsbContainer::getDeviceInfo(QString &stringi)
{
    QStringList tmp;
    size_t size = _usbDevices.size(), i;
    for(i=0; i<size ; ++i) {
        if(_usbDevices[i].getProductString() == stringi ) {
            break;
        }
    }
    if(i == size) {
        tmp.append("No such device");
    } else {
        tmp = _usbDevices[i].devInfo();
    }
    return tmp;
}

UsbDev *UsbContainer::getDevice( const QString &devInfo ) {
    auto iter = _usbDevices.begin();
    for ( int i =0; i < _usbDevices.size(); ++i, iter +=1 ) {
        if ( iter->getProductString() == devInfo ) {
            return iter; 
        }
    }
    return &deft;
}

UsbDev *UsbContainer::getDevice( const int num ) {
    int nonsudodevnr =0;
    auto iter = _usbDevices.begin();
    for ( int i =0; i < _usbDevices.size(); ++i, ++iter ) {
        if ( iter->isNonSudoDev() ) {
            if( num == nonsudodevnr ) return iter;
            ++nonsudodevnr;
        }
    }
    return &deft;
}
