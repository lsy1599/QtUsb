#include "usbdev.h"
#include <QDebug>

// Endpoint class begin
Endpoint::Endpoint()
{
    bLength           = 0;
    bDescriptorType   = 0;
    bEndpointAddress  = 0;
    bmAttributes      = 0;
    wMaxPacketSize    = 0;
    bInterval         = 0;
    bRefresh          = 0;
    bSynchAddress     = 0;
    extra_length      = 0;
}
Endpoint::Endpoint(const libusb_endpoint_descriptor *endpoint) : libusb_endpoint_descriptor(*endpoint)
{}

unsigned char Endpoint::getbEndpointAddress()
{
    return bEndpointAddress;
}

unsigned int Endpoint::decriptorSize()
{
    return bLength;
}

QString Endpoint::direction()
{
    qDebug() << "dir" << (bEndpointAddress&0x80);
    switch (bEndpointAddress&0x80) {
    case LIBUSB_ENDPOINT_IN:
        return "In";
    default:
        return "Out";
    }
}

unsigned int Endpoint::endpointNr()
{
    return bEndpointAddress&0x07;
}

unsigned int Endpoint::maxPacketSize()
{
    return wMaxPacketSize;
}
unsigned int Endpoint::pollintInterval()
{
    return bInterval;
}
unsigned int Endpoint::refreshFeedback()
{
    return bRefresh;
}
unsigned int Endpoint::synchAddress()
{
    return bSynchAddress;
}

QStringList Endpoint::parse_bmAttributes()
{
    QStringList tmp;
    QString tmpStr = "Transfer type: ";
    switch (bmAttributes&0x3) {
    case 0:
        tmpStr += "Control";
        break;
    case 1:
        tmpStr+= "Isochronous";
        break;
    case 2:
        tmpStr += "Bulk";
        break;
    case 3:
        tmpStr += "Interrupt";
        break;
    }
    tmp.append(tmpStr);

    // if isochronous
    if((bmAttributes&0x3) == 1) {
        tmpStr = "Sync. type: ";
        switch(bmAttributes&(3<<1)) {
        case 0:
            tmpStr+= "No Synchonisation";
            break;
        case 1:
            tmpStr+="Asynchronous";
            break;
        case 2:
            tmpStr+= "Adaptive";
            break;
        case 3:
            tmpStr+="Synchronous";
            break;
        }
        tmp.append(tmpStr);
        tmpStr = "Usage type: ";
        switch(bmAttributes&(2<<3)) {
        case 0:
            tmpStr+= "Data Endpoint";
            break;
        case 1:
            tmpStr+= "Feedback Endpoint";
            break;
        case 2:
            tmpStr+="Explicit Feedback Data Endpoint";
            break;
        case 3:
            tmpStr+="Reserved";
            break;
        }
        tmp.append(tmpStr);
    }
    return QStringList(tmp);
}

bool Endpoint::exist() const
{
    return bool(
               bDescriptorType       != 0xFF
               && bEndpointAddress   != 0xFF
               && wMaxPacketSize     != 0
           );
}

// Endpoint class end

// Alternate setting
AlternateSetting::AlternateSetting()
{
    interface_descriptor.bNumEndpoints = 0;
}

AlternateSetting::AlternateSetting(const libusb_interface_descriptor* toGet) : interface_descriptor(*toGet)
{

    char* tmp = new char[ toGet->extra_length + 1 ];
    strncpy(tmp,(const char*)(toGet->extra),toGet->extra_length);
    _extra = tmp;
    delete[] tmp;

    for(int i=0; i<toGet->bNumEndpoints; ++i) {
        _endpoint.push_back(toGet->endpoint+i);
    }
}

AlternateSetting::~AlternateSetting()
{   }

bool AlternateSetting::exist()
{
    return interface_descriptor.bNumEndpoints?true:false;
}

int AlternateSetting::interaceClass()
{
    return interface_descriptor.bInterfaceClass;
}

Endpoint AlternateSetting::getEndpoint(Endpoint::Direction IO)
{
    QString tmp;
    if (IO == Endpoint::Direction::In  ) tmp = "In";
    if (IO == Endpoint::Direction::Out ) tmp = "Out";
    for (int i = 0; i < _endpoint.size() ; ++i ) {
        qDebug() << "endpoint dir: " << _endpoint[i].direction();
        if( _endpoint[i].direction() == tmp ) {
            return _endpoint[i];
        }
    }
    qDebug() << "REALLY?";
    return Endpoint();
}

Endpoint AlternateSetting::getEndpoint(int nr) throw(Error)
{
    if(!(nr < interface_descriptor.bNumEndpoints)) {
        throw Error( QString(__PRETTY_FUNCTION__) +"Endpoint does not exist");
    } else {
        return _endpoint[nr];
    }
}

bool AlternateSetting::setEndpoint(Endpoint toPush)
{
    if(_endpoint.size() < interface_descriptor.bNumEndpoints) {
        _endpoint += toPush;
        return true;
    } else {
        return false;
    }
}

QString AlternateSetting::InterfaceProtocol()
{
    return USBParser::parseDeviceClass(interface_descriptor.bDescriptorType);
}

int AlternateSetting::getInterfaceNr()
{
    return interface_descriptor.bInterfaceNumber;
}
// End of alternate setting
// Interface

Interface::Interface() : _numOfAltInterfaces(0)
{}

Interface::Interface(int numOfAltInterfaces) : _numOfAltInterfaces(numOfAltInterfaces)
{}

Interface::Interface(const libusb_interface* toGet)
{
    _numOfAltInterfaces = toGet->num_altsetting;

    for(int i=0; i<_numOfAltInterfaces; ++i) {
        AlternateSetting tmpAlternateSetting(toGet->altsetting);
        _alternateSetting.push_back(tmpAlternateSetting);
    }
}

QStringList Interface::getInterfacesNames()
{
    QStringList tmp;
    for(int i=0; i<_numOfAltInterfaces; ++i) {
        tmp.append("\t->" + USBParser::parseDeviceClass(_alternateSetting[i].interaceClass()));
    }
    return tmp;
}

AlternateSetting Interface::fetchInterface(QString name)
{
    auto iter = _alternateSetting.begin();
    for( int i = 0, max=_alternateSetting.size();
            i < max;
            ++i
       ) {
        if ( name == USBParser::parseDeviceClass(iter[i].interaceClass()) ) {
            return iter[i];
        }
    }
    return AlternateSetting();
}

// End of interface

QString USBParser::parseDeviceClass(int code)
{
    switch (code) {
    case LIBUSB_CLASS_PER_INTERFACE:
        return "Interface specyfic";
    case LIBUSB_CLASS_AUDIO:
        return "Audio";
    case LIBUSB_CLASS_COMM:
        return "COM";
    case LIBUSB_CLASS_HID:
        return "HID";
    case LIBUSB_CLASS_PHYSICAL:
        return "Physical";
    case LIBUSB_CLASS_PRINTER:
        return "Printer";
    case LIBUSB_CLASS_PTP:
        return "PTP (Image)";
    case LIBUSB_CLASS_MASS_STORAGE:
        return "Mass storage";
    case LIBUSB_CLASS_DATA:
        return "Data";
    case LIBUSB_CLASS_SMART_CARD:
        return "Smart card";
    case LIBUSB_CLASS_CONTENT_SECURITY:
        return "Content security";
    case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
        return "Personal healthcare";
    case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:
        return "Diagnostic device";
    case LIBUSB_CLASS_WIRELESS:
        return "Wireless";
    case LIBUSB_CLASS_APPLICATION:
        return "Application";
    case LIBUSB_CLASS_VENDOR_SPEC:
        return "Vendor specyfic";
    case -1:
        return "ERROR";
    default:
        return "Class code not specified";
    }
}

QString USBParser::parseUsbError(int error)
{
    switch ( error) {
    case LIBUSB_SUCCESS:
        return "success";
    case LIBUSB_ERROR_IO:
        return "I/O error";
    case LIBUSB_ERROR_INVALID_PARAM:
        return "invalid param";
    case LIBUSB_ERROR_ACCESS:
        return "access denied";
    case LIBUSB_ERROR_NO_DEVICE:
        return "no device";
    case LIBUSB_ERROR_NOT_FOUND:
        return "entity not found";
    case LIBUSB_ERROR_BUSY:
        return "resource bussy";
    case LIBUSB_ERROR_TIMEOUT:
        return "timeouted";
    case LIBUSB_ERROR_OVERFLOW:
        return "overflow";
    case LIBUSB_ERROR_PIPE:
        return "pipe error";
    case LIBUSB_ERROR_INTERRUPTED:
        return "sys call interrupted";
    case LIBUSB_ERROR_NO_MEM:
        return "insufficient memory";
    case LIBUSB_ERROR_NOT_SUPPORTED:
        return "operation not supported";
    case LIBUSB_ERROR_OTHER:
        return "other error";
    default:
        return "error unspecified";
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
    } else {
        error= libusb_open(device,&_device_handle);
        if(error<0) {
            QTextStream(errorLog) << "Error: Opening USB devicenr: "
                                  << devNr
                                  << endl;
        } else {
            _isOpen = 1;
            error = libusb_get_string_descriptor_ascii(_device_handle,
                    _device_descriptor.iManufacturer,
                    string_buffer_manufacturer,
                    sizeof(string_buffer_manufacturer));
            if(error < 0) {
                QTextStream(errorLog) << "Error: Converting descriptor to ASCII [iManufacturer]" << endl;
            } else {
                //getting the ASCII text value from the descriptor field
                error = libusb_get_string_descriptor_ascii(_device_handle,
                        _device_descriptor.iProduct,
                        string_buffer_product,
                        sizeof(string_buffer_product));
                if(error<0) {
                    //reporting error on the list entry
                    QTextStream(errorLog) << "Error: Converting descriptor to ASCII [iProduct]" <<endl;
                } else {
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
                    for(int i=0; i< _numOfInterfaces; i++) {
                        Interface tmpInterface(_device_config->interface);
                        _interface.push_back(tmpInterface);
                    }
                    _deviceClass = parseDeviceClass();
                    libusb_free_config_descriptor(_device_config);

                    if(_isOpen == 1) {
                        libusb_close(_device_handle);
                        _isOpen = 0;
                    }
                }
            }
        }
        if (_isOpen == 1) {
            libusb_close(_device_handle);
            _isOpen = 0;
        }
    }
}

UsbDev::~UsbDev()
{
    close();
}

int UsbDev::open() throw(Error)
{
    if(_isOpen) {
        return true;
    }
    int tmp = libusb_open(_device, &_device_handle);
    if(tmp < 0) {
        _deviceLog << "Error: Opening USB device";
        if(_isUserConnectable != false) _isUserConnectable = false;
        _isOpen = false;
        throw Error( QString(__PRETTY_FUNCTION__) +"Can't open USB device");
    } else {
        st.push(State::ST::Opened);
        if(!_isUserConnectable) _isUserConnectable = true;
        return (_isOpen = true);
    }
}

int UsbDev::getNumOfPossibleConfigurations()
{
    return _numOfConfigurations;
}

int UsbDev::getVendorID()
{
    return _idVendor;
}
int UsbDev::getProductID()
{
    return _idProduct;
}

QStringList UsbDev::getDeviceClass()
{
    return _deviceClass;
}

libusb_device_handle* UsbDev::getHandle() throw(Error)
{
    if ( _device_handle ) {
        return _device_handle;
    } else {
        Error b("No handle available");
        throw b;
    }
}

AlternateSetting UsbDev::getInterface(QString endpointType) throw(Error)
{
    auto iter = _interface.begin();
    for( int i=0; i< _interface.size(); ++i ) {
        AlternateSetting tmp = iter[i].fetchInterface(endpointType);
        if( tmp.exist() ) {
            return tmp;
        }
    }
    if ( iter == _interface.end() ) {
        throw Error( QString(__PRETTY_FUNCTION__) +"No such interface");
    }
    // this should never get here
    return AlternateSetting();
}

// returns first endpoint of that type
unsigned char UsbDev::getEndpoint(QString endpointType, QString &result)
{
    result = "uninitialised";
    for( auto i: _interface ) {
        AlternateSetting tmp = i.fetchInterface(endpointType);
        if( tmp.exist() ) {
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

QStringList UsbDev::devInfo()
{
    QStringList retQStringList;
    retQStringList.append(QString("Product:\t\t") + getProductString());
    retQStringList.append(QString("Manufacturer:\t") + getManufacturerString());
    retQStringList.append(QString("Device class:\t"));
    retQStringList.append(getDeviceClass());
    retQStringList.append(QString("Vendor ID:\t\t") + QString("0x%1").arg(getVendorID(),0,16));
    retQStringList.append(QString("Produc ID:\t\t") + QString("0x%1").arg(getProductID(),0,16));
    retQStringList.append(QString("Num of possible configurations:\t") + QString("%1").arg(getNumOfPossibleConfigurations(),0,10));
    retQStringList.append(QString("Num of device interfaces is:\t") +QString("%1").arg(_numOfInterfaces,0,10));
    return retQStringList;
}

// TODO use IO
void raw_interrupt_transfer( libusb_device_handle *handle,
                             Endpoint::Direction IO,
                             unsigned char *buf,
                             size_t buf_len,
                             int &act_len,
                             unsigned int timeout
                           ) throw(Error)
{
    int nBytes = libusb_interrupt_transfer(  handle,
                 IO,
                 buf,
                 buf_len,
                 &act_len,
                 timeout
                                          );
    if ( nBytes < 0 )
        throw Error( QString(__PRETTY_FUNCTION__) +USBParser::parseUsbError(nBytes));
}

void raw_release_interface( libusb_device_handle *handle, uint8_t epNr ) throw(Error)
{
    int ret = libusb_release_interface( handle,epNr);
    if ( ret < 0 ) {
        throw Error( QString(__PRETTY_FUNCTION__) + USBParser::parseUsbError(ret));
    }
}

void raw_close(libusb_device_handle *handle)
{
    libusb_close(handle);
}

// TODO add data to interrutp_transfer and remove tmp set_me_ablaze
// this was for tests purposes for led blinking :)
QString UsbDev::interrupt_transfer( Endpoint::Direction IO, const unsigned int timeout=5000 )
{
    static bool set_me_ablaze = true;
    unsigned char buffer[2] = { 0xff,0xff };
    buffer[1] = set_me_ablaze;
    int act_len;

    try {
        this->open();
        AlternateSetting tmpInt = this->getInterface("HID");
        this->checkAndDetachKernelDriver(this->getHandle(),tmpInt.getInterfaceNr());
        this->claim_interface( this->getHandle(), tmpInt.getInterfaceNr() );
        this->checkBuffor( sizeof(buffer),tmpInt.getEndpoint(Endpoint::Direction::Out).maxPacketSize(), Endpoint::Direction::Out );
        raw_interrupt_transfer(this->getHandle(), IO, buffer, sizeof(buffer), act_len, timeout );
        if (set_me_ablaze == true) {
            set_me_ablaze = false;
        } else {
            set_me_ablaze = true;
        }
        freeStates();
    } catch (Error &err) {
        freeStates();
        return err.get();
    }
    return "";
}

// TODO: change 0,0, to some named values, check 2nd value in controll transfer, check if IO is actually ok
QString UsbDeb::controll_transfer(Endpoint::Direction IO, char* bufer, size_t buf_size, const unsigned int timeout=5000)
{
    libusb_control_transfer(
        this->getHandle(),
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        IO,
        0,
        0,
        bufer,
        buf_size,
        timeout
    );
}

int UsbDev::close()
{
    if(_isOpen) {
        raw_close(this->getHandle());
        return (_isOpen = false);
    } else {
        return (_isOpen = true);
    }
}

void UsbDev::deClaim() throw(Error)
{
    raw_release_interface(this->getHandle(), st.popInNr() );
}
bool UsbDev::freeStates()
{
    try {
        State::ST tmp = st.pop();
        for (; tmp != State::ST::None; tmp = st.pop() ) {
            switch ( tmp ) {
            case State::ST::None:
                break;
            case State::ST::Opened:
                close();
                break;
            case State::ST::Claimed:
                deClaim();
                break;
            default:
                qDebug() << "Success!";
                break;
            }
        }
    } catch ( Error &err ) {
        qDebug() << "What do do now?";
        qDebug() << err.get();
        return false;
    }
    return true;
}

// USB dev - private

QStringList UsbDev::parseDeviceClass()
{
    QStringList tmp;
    if(_device_descriptor.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE) {
        for(int i=0; i<_device_descriptor.bNumConfigurations; ++i) {
            tmp.append(_interface[i].getInterfacesNames());
        }
    } else {
        tmp.append(USBParser::parseDeviceClass(_device_descriptor.bDeviceClass));
    }
    return tmp;
}

void UsbDev::checkAndDetachKernelDriver(libusb_device_handle* handle, uint8_t interfaceNr )
{
    if ( libusb_kernel_driver_active(handle, interfaceNr ) == true ) {  //tmpInt.getInterfaceNr()
        libusb_detach_kernel_driver(handle, interfaceNr );
    }
}

/// TODO:
/// Change so that IO in both ways will be ok -
/// so that: when send size < buffSize (and pass send size )
/// can be checked with ?strlen? no if we want to send zeroes
void UsbDev::checkBuffor( size_t buffSize, size_t eBuffSize, Endpoint::Direction IO ) throw(Error)
{
    if ( buffSize > eBuffSize && IO == Endpoint::Direction::In ) {
        throw Error( QString(__PRETTY_FUNCTION__) +"Buffor too big for transfer");
    } else if (buffSize < eBuffSize && IO == Endpoint::Direction::Out ) {
        throw Error( QString(__PRETTY_FUNCTION__) +"Buffor too small for transfer");
    }
}

void UsbDev::claim_interface(libusb_device_handle* handle, unsigned int epNr ) throw(Error)
{
    if ( libusb_claim_interface(handle,epNr)  < 0 ) { //tmpInt.getEndpoint(AlternateSetting::Direction::In).endpointNr());
        throw Error( QString(__PRETTY_FUNCTION__) +"Can not claim interface");
    } else {
        st.push(epNr);
    }
}
