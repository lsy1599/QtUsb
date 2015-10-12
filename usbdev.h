#ifndef USBDEV_H
#define USBDEV_H

#include <libusb-1.0/libusb.h>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <QTextStream>
#include <QVector>


// TODO when HID class is properly interfaced, than I can check all it properties with
// Endpoint and then Write & Read from it properly with UsbHidDevice

namespace USBParser{
    QString parseDeviceClass(int whatIs);
}

// It was struct - but to avoid warnings considering default 0xFF values which are far off
// Values which could be gained during usage (f.e. max adress - 127) constructor had to be
// created
class Endpoint{
private:
    libusb_endpoint_descriptor _endpoint;
public:

    Endpoint();
    Endpoint(const libusb_endpoint_descriptor *endpoint);

    unsigned char getbEndpointAddress()
    {
        return _endpoint.bEndpointAddress;
    }

    unsigned int decriptorSize(){ return _endpoint.bLength; }
    // to check what is that in endpoint uint8_t  bDescriptorType;
    // returns In / Out string
    QString direction(){
        switch (_endpoint.bEndpointAddress&0x80) {
        case LIBUSB_ENDPOINT_IN:
            return "In";
            break;
        default:
            return "Out";
            break;
        }
    }

    unsigned int endpointNr(){
        return _endpoint.bEndpointAddress&0x07;
    }

    unsigned int maxPacketSize(){ return _endpoint.wMaxPacketSize; }
    unsigned int pollintInterval(){ return _endpoint.bInterval; }
    unsigned int refreshFeedback(){ return _endpoint.bRefresh; }
    unsigned int bSynchAddress(){ return _endpoint.bSynchAddress; }

    QString extra(){
        char* tmp = new char[ _endpoint.extra_length + 1 ];
        strncpy(tmp,(const char*)_endpoint.extra,_endpoint.extra_length);
        return QString(tmp);
    }  // int extra_length strncpy shal be used better

    QStringList parse_bmAttributes(){
        QStringList tmp; QString tmpStr = "Transfer type: ";
        switch (_endpoint.bmAttributes&0x3) {
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
        if((_endpoint.bmAttributes&0x3) == 1){
            tmpStr = "Sync. type: ";
            switch(_endpoint.bmAttributes&(3<<1)){
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
            switch(_endpoint.bmAttributes&(2<<3)){
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
        return tmp;
    }
};

class AlternateSetting{
private:

    uint8_t _bNumEndpoints;
//    AltSetStruct _altSeting;
    QVector<Endpoint> _endpoint;
    libusb_interface_descriptor _libusb_interface_descriptor;
    QString _extra;
public:

    AlternateSetting();
    AlternateSetting(const libusb_interface_descriptor* toGet);
    ~AlternateSetting();

    bool exist(){
        return _bNumEndpoints?true:false;
    }

    int interaceClass();

    Endpoint getEndpoint(int nr) const;
    bool setEndpoint(Endpoint toPush);
    QString InterfaceProtocol(){
        return USBParser::parseDeviceClass(_libusb_interface_descriptor.bDescriptorType);
    }
};

// TODO return whole information about all interfaces accessible

class Interface{
private:
    int _numOfAltInterfaces;
    int _interfaceNr;
    QVector<AlternateSetting> _alternateSetting;

public:
    Interface() : _numOfAltInterfaces(0) {}
    Interface(int numOfAltInterfaces) : _numOfAltInterfaces(numOfAltInterfaces) {}
    Interface(const libusb_interface* toGet){
        _numOfAltInterfaces = toGet->num_altsetting;

        for(int i=0;i<_numOfAltInterfaces;++i){
            AlternateSetting tmpAlternateSetting(toGet->altsetting);
            _alternateSetting.push_back(tmpAlternateSetting);
        }
    }

    QStringList getInterfacesNames(){
        QStringList tmp;
        for(int i=0;i<_numOfAltInterfaces;++i){
            tmp.append("\t->" + USBParser::parseDeviceClass(_alternateSetting[i].interaceClass()));
        }
        return tmp;
    }

    AlternateSetting fetchInterface(QString name){
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

};

class UsbDev
{
    enum{
        OUT_OF_USB_BUS = 128,
    };

public:
    UsbDev();                                       //to create vector in container
    UsbDev(libusb_device *device, int devNr, QString *errorLog);
    ~UsbDev();

    virtual QString write(){ return QString("Base class - can't write"); }
    int open();
    int close();

    int getNumOfPossibleConfigurations();
    QStringList getDeviceClass();
    int getVendorID();
    int getProductID();
    int getConfigDescriptor();

    libusb_device_handle *getHandle(){ return _device_handle; }
    // returns first endpoint of that type
    unsigned char getEndpoint(QString endpointType, QString &result){
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

    bool isNonSudoDev() const{ return _nonSUdoDev; }
    QString getProductString() const{ return _product; }
    QString getManufacturerString() const{ return _manufacturer; }
    QStringList devInfo();
    QStringList getConfigData();

private:
    int _isOpen;                                    //is device connected
    int _deviceNumber;                              //device number on USB bus
    bool _nonSUdoDev;                               //device usable without sudo
    libusb_device           *_device;               //libusb device
    libusb_device_handle    *_device_handle;        //handle to USB device
    libusb_device_descriptor _device_descriptor;    //the device descriptor
    libusb_config_descriptor *_device_config;        //the config descriptor

    QVector<Interface> _interface;

    QStringList _deviceClass;
    QString _manufacturer;                          //for storing manufacturer descriptor
    QString _product;                               //for storing product descriptor

    int _numOfConfigurations;
    int _idVendor;
    int _idProduct;

    int _numOfInterfaces;
    int _numberOfAlternateSettings;
    int _numberOfEndpoints;

    bool _isUserConnectable;
    QStringList _deviceLog;

    QStringList parseDeviceClass();
};

class UsbHidDev : public UsbDev{
public:
   UsbHidDev(const UsbDev &parentUsbDev) : UsbDev(parentUsbDev) {}
   QString write();
};

#endif // USBDEV_H
