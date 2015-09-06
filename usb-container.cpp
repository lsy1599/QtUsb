#include "usb-container.h"

#define SHITHAPPENS 0 // get from project this shit

UsbContainer::UsbContainer(QString *errorLog) : _error(0), _numOfDev(0), _errorLog(errorLog)
{
    _error = usbLibInit();
    if(_error < 0) QTextStream(_errorLog) << "Error: Initializing libusb" << endl;
    else{
        _numOfDev = libusb_get_device_list(_ctx, &_device_list);
        if(_numOfDev < 1) {
            QTextStream(_errorLog) << "Error: No dev found" << endl;
            return;
        }else{
            for(uint i=0;i<_numOfDev; i++)
            {
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

QStringList UsbContainer::listNonRootDevices(){
    QStringList tmpList;
    for(int i=0;i<_usbDevices.size();i++)
    {
        if(_usbDevices.at(i).isNonSudoDev()){
            tmpList.append(_usbDevices.at(i).getProductString() +"\t" +_usbDevices.at(i).getManufacturerString()); }
    }
    return tmpList;
}

int UsbContainer::writeToDevice(uint i,QString *ohShiet){
    QTextStream(ohShiet) << _usbDevices[i].write();

//    int errorCode = libusb_open(*_device_list, (libusb_device_handle**)&(_usbDevices.at(i)._device_handle));
//    if(errorCode<0){
//        QTextStream(ohShiet) << "Connection failure " <<endl;
//        return errorCode;
//    }

//////    dev.ctrl_transfer(0x40, 0, button.get_active(), 0, 'Hello World!')

//    unsigned char bOut[] = "test";
//    int result = libusb_control_transfer(
//        _usbDevices.at(i)._device_handle,
//        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, // bRequestType
//        SHITHAPPENS,                                          // bRequest
//        0,                                                    // wValue
//        0,                                                    // wIndex
//        bOut,                                                 // pointer to destination buffer
//        sizeof(bOut),                                         // wLength
//        100                                                   // timeout
//        );

//    if(ohShiet){
//        QTextStream(ohShiet) << "przesłano bajtów:" << result << endl;
//    }else{
//        QTextStream(ohShiet) << "Nie przesłano bajtów:" << result << endl;
//    }
////    // int libusb_claim_interface
////    //int libusb_release_interface
////    //void libusb_close
////    return errorCode;
}

