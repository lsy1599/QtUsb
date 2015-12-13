#include "usb-container.h"

#define SHITHAPPENS 0 // get from project this shit

UsbContainer::UsbContainer(QString *errorLog) : _error(0), _numOfDev(0), _errorLog(errorLog)
{
    _error = usbLibInit();
    if(_error < 0) QTextStream(_errorLog) << "Error: Initializing libusb" << endl;
    else {
        _numOfDev = libusb_get_device_list(_ctx, &_device_list);
        if (_numOfDev < 1) {
            QTextStream(_errorLog) << "Error: No dev found" << endl;
            return;
        }else{
            for (uint i=0;i<_numOfDev; i++) {
                _usbDevices.push_back(UsbDev(_device_list[i],i,errorLog));
            }
        }
    }
}

UsbContainer::~UsbContainer(){
    libusb_exit(_ctx);
}

int UsbContainer::usbLibInit(){
    uint error;
    return error = libusb_init(&_ctx);
}


// Lists all devices which are accesible with no root privileges
QStringList UsbContainer::listNonRootDevices(){
    QStringList tmpList;
    for(int i=0;i<_usbDevices.size();i++)
    {
        if(_usbDevices.at(i).isNonSudoDev()){
            tmpList.append(_usbDevices.at(i).getProductString());
            // +"\t" +_usbDevices.at(i).getManufacturerString());
        }
    }
    return tmpList;
}

QString UsbContainer::writeToDevice(QString &productString){
    size_t size = _usbDevices.size() , i;
    for(i = 0; i<size; ++i){
        if(_usbDevices[i].getProductString() == productString){
            break;
        }
    }
    if(i == size){
        // here shall be some kind of error to be more informative
        // one shall give string not int, or the best error clas
        return "One shall not pass";
    }else{
        return _usbDevices[i].interrupt_transfer(Endpoint::Direction::Out);
    }
}

QStringList UsbContainer::getDeviceInfo(int i){
    QString tmp;
    QTextStream(&tmp) << _usbDevices.size();
    QStringList tmpList;
    tmpList.append(tmp);
    QTextStream(&tmp) << i;
    tmpList.append(tmp);
    return tmpList;
}

QStringList UsbContainer::getDeviceInfo(QString &stringi){
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

