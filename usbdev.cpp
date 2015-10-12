#include "usbdev.h"

// Endpoint class begin
Endpoint::Endpoint(){
    _endpoint.bDescriptorType = 0xFF;
    _endpoint.bEndpointAddress = 0xFF;
    _endpoint.wMaxPacketSize =(0);
}
Endpoint::Endpoint(const libusb_endpoint_descriptor *endpoint) : _endpoint(*endpoint)
{}
// Endpoint class end

// Alternate setting
AlternateSetting::AlternateSetting(): _bNumEndpoints(0){}
AlternateSetting::AlternateSetting(const libusb_interface_descriptor* toGet) : _libusb_interface_descriptor(*toGet){

    _bNumEndpoints = toGet->bNumEndpoints;
    char* tmp = new char[ toGet->extra_length + 1 ];
    strncpy(tmp,(const char*)(toGet->extra),toGet->extra_length);
    _extra = tmp;

    for(int i=0;i<_bNumEndpoints;++i){
        Endpoint tmpEndpoint(toGet->endpoint);
        _endpoint.push_back(tmpEndpoint);
    }
}

AlternateSetting::~AlternateSetting()
{   }

int AlternateSetting::interaceClass(){
    return _libusb_interface_descriptor.bInterfaceClass;
}

// not checked:
Endpoint AlternateSetting::getEndpoint(int nr) const{
    if(!(nr < _bNumEndpoints)){
        return Endpoint();
    }else{
        return _endpoint[nr];
    }
}

bool AlternateSetting::setEndpoint(Endpoint toPush){
    if(_endpoint.size() < _bNumEndpoints){
        _endpoint += toPush;
        return true;
    }else{
        return false;
    }
}

// End of alternate setting

QString USBParser::parseDeviceClass(int whatIs){
    switch (whatIs) {
        case LIBUSB_CLASS_PER_INTERFACE:
            return "Interface specyfic";
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

UsbDev::UsbDev() :  _isOpen(false),
                    _deviceNumber(OUT_OF_USB_BUS),
                    _nonSUdoDev(0),
                    _device(0),
                    _device_handle(0)
{ }

UsbDev::UsbDev(libusb_device *device,int devNr,QString *errorLog) : _isOpen(false),
                                                                    _nonSUdoDev(false),
                                                                    _device(device)
{
//    if( getDeviceDescriptor() != true){return;}
//    if( open() != true ){return;}
//    if( getManufacturer() != true ){return;}
//    if( getProduct() != true ){return; }
//    if( close() != true ){ return; }
//}


    unsigned char string_buffer_manufacturer[4096];     //for storing manufacturer descriptor
    unsigned char string_buffer_product[4096];          //for storing product descriptor

    int error=0;
    error = libusb_get_device_descriptor(device, &_device_descriptor);
    if(error < 0) {
        QTextStream(errorLog) << "Error: Failed to get descriptor"<< endl;
    }else{
        error= libusb_open(device,&_device_handle);
        if(error<0){
            QTextStream(errorLog) << "Error: Opening USB devicenr: "
                                  << devNr
                                  << endl;
        }else{
            _isOpen = 1;
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
                    _numOfConfigurations = _device_descriptor.bNumConfigurations;
                    _idVendor = _device_descriptor.idVendor;
                    _idProduct = _device_descriptor.idProduct;

                    libusb_get_config_descriptor(_device, 0, &_device_config);
                    _numOfInterfaces = _device_config->bNumInterfaces;
                    for(int i=0; i< _numOfInterfaces; i++){
                        Interface tmpInterface(_device_config->interface);
                        _interface.push_back(tmpInterface);
                    }
                    _deviceClass = parseDeviceClass();
                    libusb_free_config_descriptor(_device_config);

                    if(_isOpen == 1){
                        libusb_close(_device_handle);
                        _isOpen = 0;
                    }
                }
            }
        }
        if (_isOpen == 1){
            libusb_close(_device_handle);
            _isOpen = 0;
        }
    }
}

UsbDev::~UsbDev(){
    close();
}

int UsbDev::open(){
    if(_isOpen){
        return true;
    }

    int tmp = libusb_open(_device, &_device_handle);

    if(tmp < 0){
         _deviceLog << "Error: Opening USB device";
        if(_isUserConnectable != false) _isUserConnectable = false;
        return (_isOpen = false);
    }else{
        if(!_isUserConnectable) _isUserConnectable = true;
        return (_isOpen = true);
    }
}

int UsbDev::close(){
    if(_isOpen){
        libusb_close(_device_handle);
        return (_isOpen = false);
    }else{
        return (_isOpen = true);
    }
}

QStringList UsbDev::parseDeviceClass(){
    QStringList tmp;
    if(_device_descriptor.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE){
        for(int i=0;i<_device_descriptor.bNumConfigurations;++i){
            tmp.append(_interface[i].getInterfacesNames());
        }
    }else{
        tmp.append(USBParser::parseDeviceClass(_device_descriptor.bDeviceClass));
    }
    return tmp;
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

QStringList UsbDev::getDeviceClass(){
    return _deviceClass;
}

QStringList UsbDev::devInfo(){
    QStringList retQStringList;
    retQStringList.append(QString("Product:\t\t") + getProductString());
    retQStringList.append(QString("Manufacturer:\t") + getManufacturerString());
    retQStringList.append(QString("Device class:\t")); retQStringList.append(getDeviceClass());
    retQStringList.append(QString("Vendor ID:\t\t") + QString("0x%1").arg(getVendorID(),0,16));
    retQStringList.append(QString("Produc ID:\t\t") + QString("0x%1").arg(getProductID(),0,16));
    retQStringList.append(QString("Num of possible configurations:\t") + QString("%1").arg(getNumOfPossibleConfigurations(),0,10));
    retQStringList.append(QString("Num of device interfaces is:\t") +QString("%1").arg(_numOfInterfaces,0,10));
    return retQStringList;
}

QString UsbHidDev::write(){
    unsigned char buffer[64];
    unsigned int timeout = 5000;
    int nBytes, act_len;
    QString result;
    unsigned char endpoint = this->getEndpoint("HID",result);
    if(result == "success")
    {
        nBytes = libusb_interrupt_transfer( this->getHandle(),
                                                endpoint,
                                                buffer,
                                                sizeof(buffer),
                                                &act_len,
                                                timeout
                                              );
    }
    return(QString("Not tested yet %1").arg(nBytes,0,10));
}
