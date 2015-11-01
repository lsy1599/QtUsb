#include "usbdev.h"

// Endpoint class begin
Endpoint::Endpoint() {
    _endpoint.bDescriptorType = 0xFF;
    _endpoint.bEndpointAddress = 0xFF;
    _endpoint.wMaxPacketSize =(0);
}
Endpoint::Endpoint(const libusb_endpoint_descriptor *endpoint) : _endpoint(*endpoint)
{}

unsigned char Endpoint::getbEndpointAddress()
{
    return _endpoint.bEndpointAddress;
}

unsigned int Endpoint::decriptorSize(){ return _endpoint.bLength; }

QString Endpoint::direction() {
    switch (_endpoint.bEndpointAddress&0x80) {
        case LIBUSB_ENDPOINT_IN: return "In";
        default: return "Out"; break;
    }
}

unsigned int Endpoint::endpointNr() {
    return _endpoint.bEndpointAddress&0x07;
}

unsigned int Endpoint::maxPacketSize(){ return _endpoint.wMaxPacketSize; }
unsigned int Endpoint::pollintInterval(){ return _endpoint.bInterval; }
unsigned int Endpoint::refreshFeedback(){ return _endpoint.bRefresh; }
unsigned int Endpoint::bSynchAddress(){ return _endpoint.bSynchAddress; }

QString Endpoint::extra() {
    char* tmp = new char[ _endpoint.extra_length + 1 ];
    strncpy(tmp,(const char*)_endpoint.extra,_endpoint.extra_length);
    extra_ = tmp;
    delete[] tmp;
    return QString(extra_);
}

QStringList Endpoint::parse_bmAttributes(){
    QStringList tmp; QString tmpStr = "Transfer type: ";
    switch (_endpoint.bmAttributes&0x3) {
        case 0: tmpStr += "Control"; break;
        case 1: tmpStr+= "Isochronous"; break;
        case 2: tmpStr += "Bulk"; break;
        case 3: tmpStr += "Interrupt"; break;
    }
    tmp.append(tmpStr);

    // if isochronous
    if((_endpoint.bmAttributes&0x3) == 1){
        tmpStr = "Sync. type: ";
        switch(_endpoint.bmAttributes&(3<<1)){
            case 0: tmpStr+= "No Synchonisation"; break;
            case 1: tmpStr+="Asynchronous"; break;
            case 2: tmpStr+= "Adaptive"; break;
            case 3: tmpStr+="Synchronous"; break;
        }
        tmp.append(tmpStr);
        tmpStr = "Usage type: ";
        switch(_endpoint.bmAttributes&(2<<3)){
            case 0: tmpStr+= "Data Endpoint"; break;
            case 1: tmpStr+= "Feedback Endpoint"; break;
            case 2: tmpStr+="Explicit Feedback Data Endpoint"; break;
            case 3: tmpStr+="Reserved"; break;
        }
        tmp.append(tmpStr);
    }
    return QStringList(tmp);
}

bool Endpoint::exist() const
{
    return bool(
            _endpoint.bDescriptorType       != 0xFF
            && _endpoint.bEndpointAddress   != 0xFF
            && _endpoint.wMaxPacketSize     != 0
        );
}

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

bool AlternateSetting::exist(){
    return _bNumEndpoints?true:false;
}

int AlternateSetting::interaceClass(){
    return _libusb_interface_descriptor.bInterfaceClass;
}

Endpoint AlternateSetting::getEndpoint(int nr) const{
    if(!(nr < _bNumEndpoints)){
        return Endpoint();
    }else{
        return _endpoint[nr];
    }
}

bool AlternateSetting::setEndpoint(Endpoint toPush){
    if(_endpoint.size() < _bNumEndpoints) {
        _endpoint += toPush;
        return true;
    }else{
        return false;
    }
}

QString AlternateSetting::InterfaceProtocol(){
    return USBParser::parseDeviceClass(_libusb_interface_descriptor.bDescriptorType);
}
// End of alternate setting
// Interface

Interface::Interface() : _numOfAltInterfaces(0)
{}

Interface::Interface(int numOfAltInterfaces) : _numOfAltInterfaces(numOfAltInterfaces)
{}

Interface::Interface(const libusb_interface* toGet){
    _numOfAltInterfaces = toGet->num_altsetting;

    for(int i=0;i<_numOfAltInterfaces;++i){
        AlternateSetting tmpAlternateSetting(toGet->altsetting);
        _alternateSetting.push_back(tmpAlternateSetting);
    }
}

QStringList Interface::getInterfacesNames(){
    QStringList tmp;
    for(int i=0;i<_numOfAltInterfaces;++i){
        tmp.append("\t->" + USBParser::parseDeviceClass(_alternateSetting[i].interaceClass()));
    }
    return tmp;
}

AlternateSetting Interface::fetchInterface(QString name){
    auto iter = _alternateSetting.begin();
    for( int i = 0, max=_alternateSetting.size();
         i < max;
         ++i
         )
    {
        if ( name == USBParser::parseDeviceClass(iter[i].interaceClass()) )
        {
            return iter[i];
        }
    }
    return AlternateSetting();
}

// End of interface

QString USBParser::parseDeviceClass(int whatIs){
    switch (whatIs) {
            case LIBUSB_CLASS_PER_INTERFACE:        return "Interface specyfic";
            case LIBUSB_CLASS_AUDIO:                return "Audio";
            case LIBUSB_CLASS_COMM:                 return "COM";
            case LIBUSB_CLASS_HID:                  return "HID";
            case LIBUSB_CLASS_PHYSICAL:             return "Physical";
            case LIBUSB_CLASS_PRINTER:              return "Printer";
            case LIBUSB_CLASS_PTP:                  return "PTP (Image)";
            case LIBUSB_CLASS_MASS_STORAGE:         return "Mass storage";
            case LIBUSB_CLASS_DATA:                 return "Data";
            case LIBUSB_CLASS_SMART_CARD:           return "Smart card";
            case LIBUSB_CLASS_CONTENT_SECURITY:     return "Content security";
            case LIBUSB_CLASS_PERSONAL_HEALTHCARE:  return "Personal healthcare";
            case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:    return "Diagnostic device";
            case LIBUSB_CLASS_WIRELESS:             return "Wireless";
            case LIBUSB_CLASS_APPLICATION:          return "Application";
            case LIBUSB_CLASS_VENDOR_SPEC:          return "Vendor specyfic";
            case -1:                                return "ERROR";
            default:                                return "Class code not specified";
        }
}

QString USBParser::parseUsbError(int whatIs){
    switch ( whatIs) {
        case LIBUSB_SUCCESS:                return "success";
        case LIBUSB_ERROR_IO:               return "I/O error";
        case LIBUSB_ERROR_INVALID_PARAM:    return "invalid param";
        case LIBUSB_ERROR_ACCESS:           return "access denied";
        case LIBUSB_ERROR_NO_DEVICE:        return "no device";
        case LIBUSB_ERROR_NOT_FOUND:        return "entity not found";
        case LIBUSB_ERROR_BUSY:             return "resource bussy";
        case LIBUSB_ERROR_TIMEOUT:          return "timeouted";
        case LIBUSB_ERROR_OVERFLOW:         return "overflow";
        case LIBUSB_ERROR_PIPE:             return "pipe error";
        case LIBUSB_ERROR_INTERRUPTED:      return "sys call interrupted";
        case LIBUSB_ERROR_NO_MEM:           return "insufficient memory";
        case LIBUSB_ERROR_NOT_SUPPORTED:    return "operation not supported";
        case LIBUSB_ERROR_OTHER:            return "other error";
        default:                            return "error unspecified";
    };
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

libusb_device_handle* UsbDev::getHandle()
{
    return _device_handle;
}

AlternateSetting UsbDev::getInterface(QString endpointType)
{
    for( auto i: _interface )
    {
        AlternateSetting tmp = i.fetchInterface(endpointType);
        if( tmp.exist() )
        {
            return tmp;
        }
    }
}

// returns first endpoint of that type
unsigned char UsbDev::getEndpoint(QString endpointType, QString &result){
    result = "uninitialised";
    for( auto i: _interface )
    {
        AlternateSetting tmp = i.fetchInterface(endpointType);
        if( tmp.exist() )
        {
            result = "success";
            return tmp.getEndpoint(0).getbEndpointAddress();
        }
    }
    result == "fail";
    unsigned char a='\0';
    return a;
}

bool UsbDev::isNonSudoDev() const
{
    return _nonSUdoDev;
}

QString UsbDev::getProductString() const
{
    return _product;
}

QString UsbDev::getManufacturerString() const
{
    return _manufacturer;
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

QString UsbDev::write(){
    this->open();
    unsigned char buffer[2];
    unsigned int timeout = 5000;
    int nBytes, act_len;
    QString result;

//    Interface tmpInterface = this->getInterface("HID");
    unsigned char endpoint = this->getEndpoint("HID",result);
    if(result == "success" && this->open() )
    {
        nBytes = libusb_interrupt_transfer( this->getHandle(),
                                            endpoint,
                                            buffer,
                                            sizeof(buffer),
                                            &act_len,
                                            timeout
                                           );
    }
    else
    {
        return "Shit happens: " + result;
    }
    this->close();

    QString returnable("In tests : ");
    if(nBytes <0 )
    {
        QTextStream(&returnable) << USBParser::parseUsbError(nBytes);
    }
    else
    {
        QTextStream(&returnable) << "success!";
    }
    return returnable;
}

// USB dev - private

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

// UsbHid - to delete.

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
