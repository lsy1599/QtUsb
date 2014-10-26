#include "usbdev.h"

UsbDev::UsbDev() :_isConnected(false),_deviceNumber(OUT_OF_USB_BUS), _nonSUdoDev(0), _device(0), _device_handle(0){
}

UsbDev::UsbDev(libusb_device *device,int devNr,QString *errorLog) : _nonSUdoDev(false)
{
    unsigned char string_buffer_manufacturer[4096];     //for storing manufacturer descriptor
    unsigned char string_buffer_product[4096];          //for storing product descriptor

    int error=0;
    error = libusb_get_device_descriptor(device, &_device_descriptor);
    if(error < 0) {QTextStream(errorLog) << "Error: Failed to get descriptor"<< endl;
    }else{
        error= libusb_open(device,&_device_handle);
        if(error<0){ QTextStream(errorLog) << "Error: Opening USB devicenr: "<< devNr << endl;
        }else{
            _isConnected = 1;
            error = libusb_get_string_descriptor_ascii(_device_handle,
                                                       _device_descriptor.iManufacturer,
                                                       string_buffer_manufacturer,
                                                       sizeof(string_buffer_manufacturer));
            if(error < 0){
                QTextStream(errorLog) << "Error: Converting descriptor to ASCII [iManufacturer]" << endl;
            }else{
                //getting the ASCII text value from the descriptor field
                error = libusb_get_string_descriptor_ascii(_device_handle,
                                                           _device_descriptor.iProduct,
                                                           string_buffer_product,
                                                           sizeof(string_buffer_product));
                if(error<0){
                    //reporting error on the list entry
                    QTextStream(errorLog) << "Error: Converting descriptor to ASCII [iProduct]" <<endl;
                }else{
                    QTextStream(errorLog) << "Succes! Device added!" << endl;
                    QTextStream(&_manufacturer) << (const char*)string_buffer_manufacturer;
                    QTextStream(&_product) << (const char*)string_buffer_product;
                    _deviceNumber = devNr;
                    _device =device;
                    _nonSUdoDev=true;
                    _numOfConfigurations = (int) _device_descriptor.bNumConfigurations;
                    _idVendor = (int)_device_descriptor.idVendor;
                    _idProduct = (int) _device_descriptor.idProduct;

// freshest added part
                    _deviceClass = parseDeviceClass();
                    libusb_get_config_descriptor(_device, 0, &_device_config);
                    _numOfInterfaces = _device_config->bNumInterfaces;
                    for(int i=0; i< _numOfInterfaces; i++){
                        //Tu totalna zmiana
                        Interface tmpInterface(_device_config->interface);
                        _interface.push_back(&tmpInterface);
//                        _interface.push_back(&(_device_config->interface[i]));
//                        _numberOfAlternateSettings = _interfaces->num_altsetting;
//                        for(int ii=0;ii<_numberOfAlternateSettings;++ii){
//                        }
                    }
                    libusb_free_config_descriptor(_device_config);
// end of freshest part

                    if(_isConnected == 1){
                        libusb_close(_device_handle);
                        _isConnected = 0;
                    }
                }
            }
        }
        if (_isConnected == 1){
            libusb_close(_device_handle);
            _isConnected = 0;
        }
    }
}

UsbDev::~UsbDev(){
    if(_isConnected){
        deviceClose();
    }
}

int UsbDev::deviceOpen(){
    return false;
}

int UsbDev::deviceClose(){
    return false;
}

QString UsbDev::parseDeviceClass(){
    int whatIs = _device_descriptor.bDeviceClass;
    switch (whatIs) {
    case LIBUSB_CLASS_PER_INTERFACE:
        return "Per interface";
        break;
    case LIBUSB_CLASS_AUDIO:
        return "Audio";
        break;
    case LIBUSB_CLASS_COMM:
        return "COM";
        break;
    case LIBUSB_CLASS_HID:
        return "HID";
        break;
    case LIBUSB_CLASS_PHYSICAL:
        return "Physical";
        break;
    case LIBUSB_CLASS_PRINTER:
        return "Printer";
        break;
    case LIBUSB_CLASS_PTP:
        return "PTP (Image)";
        break;
    case LIBUSB_CLASS_MASS_STORAGE:
        return "Mass storage";
        break;
    case LIBUSB_CLASS_DATA:
        return "Data";
        break;
    case LIBUSB_CLASS_SMART_CARD:
        return "Smart card";
        break;
    case LIBUSB_CLASS_CONTENT_SECURITY:
        return "Content security";
        break;
    case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
        return "Personal healthcare";
        break;
    case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:
        return "Diagnostic device";
        break;
    case LIBUSB_CLASS_WIRELESS:
        return "Wireless";
        break;
    case LIBUSB_CLASS_APPLICATION:
        return "Application";
        break;
    case LIBUSB_CLASS_VENDOR_SPEC:
        return "Vendor specyfic";
        break;
    case -1:
        return "ERROR";
        break;
    default:
        return "Class code not specified";
        break;
    }
}

int UsbDev::getNumOfPossibleConfigurations(){
    return _numOfConfigurations;
}

int UsbDev::getVendorID(){
    return _idVendor;
}
int UsbDev::getProductID(){
    return _idProduct;
}

QString UsbDev::getDeviceClass(){
    return _deviceClass;
}

QStringList UsbDev::devInfo(){
    QStringList retQStringList;
    retQStringList.append("Num of possible configurations: " + getNumOfPossibleConfigurations());
    retQStringList.append("Device class: " + getDeviceClass());
    retQStringList.append("Vendor ID: " + getVendorID());
    retQStringList.append("Produc ID: " + getProductID());
    retQStringList.append("Num of device interfaces is: " + _numOfInterfaces);

//    for(int i=0; i< _numOfInterfaces; i++){
//        retQStringList.append("Interface nr:" + i);
//        retQStringList.append("\t\tNumber of alternate interface settings" + ._interfaces.at(i)._numberOfAlternateSettings);
//        for(int ii=0;ii<numOfAltSettings;++ii){
//            intdescriptor = &(inter->altsetting[ii]);
//            retQStringList.append("\t\t\tAlternate interface setting nr: " + (int)intdescriptor->bInterfaceNumber);
//            int numOfEndpoints = intdescriptor->bNumEndpoints;
//            retQStringList.append("\t\t\tNum of endpoints: " + numOfEndpoints);
//            for(int iii=0;iii<numOfEndpoints;++iii){
//                endpdesc = &intdescriptor->endpoint[iii];
//                int descriptorType = endpdesc->bDescriptorType;
//                if(descriptorType == LIBUSB_ENDPOINT_IN){
//                    retQStringList.append("\t\t\t\tDescriptor type: Input" );
//                }else if(descriptorType ==  LIBUSB_ENDPOINT_OUT){
//                    retQStringList.append("\t\t\t\tDescriptor type: Output" );
//                }else{
//                    retQStringList.append("\t\t\t\tDescriptor type: ERROR" );
//                }
//                retQStringList.append("\t\t\t\tEP Address: " + (int)endpdesc->bEndpointAddress);
//            }
//        }
//    }
    return retQStringList;
}
