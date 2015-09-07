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
public:
    int type;
    int adress;
    uint8_t interval;
    uint8_t lenght;
    uint8_t attributes;
    uint8_t refresh;
    uint8_t synchAddress;
    QString extra;
    uint16_t wMaxPacketSize;

    Endpoint();
    Endpoint(const libusb_endpoint_descriptor *endpoint);
};

struct AltSetStruct{
    uint8_t bAlternateSetting;
    uint8_t bDescriptorType;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceNumber;
    uint8_t bInterfaceProtocol;
    uint8_t bInterfaceSubClass;
    uint8_t bLength;
    QString _extra;
};

class AlternateSetting{
private:

    uint8_t _bNumEndpoints;
    AltSetStruct _altSeting;
    QVector<Endpoint> _endpoint;

public:

    AlternateSetting();
    AlternateSetting(int numOfEndpoints, AltSetStruct altSetting);
    AlternateSetting(const libusb_interface_descriptor* toGet);
    ~AlternateSetting();

    int interaceClass();

    Endpoint getEndpoint(int nr) const;
    bool setEndpoint(Endpoint toPush);
};

// TODO return whole information about all interfaces accessible

class Interface{
private:
    int _numOfAltInterfaces;
    int _interfaceNr;
    int _enumerator;
    QVector<AlternateSetting> _alternateSetting;

public:
    Interface() : _numOfAltInterfaces(0), _enumerator(0)
    {}
    Interface(int numOfAltInterfaces) : _numOfAltInterfaces(numOfAltInterfaces), _enumerator(0)
    {}
    Interface(const libusb_interface* toGet) :_enumerator(0) {
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

//    bool addInterface(AlternateSetting toPush){
//        if(_enumerator<_numOfAltInterfaces){
//            _alternateSetting[_enumerator]=toPush;
//            ++_enumerator;
//            return true;
//        }else{
//            return false;
//        }
//    }

//    AlternateSetting removeInterface(){
//        if(_enumerator >= 0){
//            AlternateSetting tmp = _alternateSetting[_enumerator];
//            _enumerator--;
//            return tmp;
//        }else{
//            AlternateSetting tmp;
//            return tmp;
//        }
//    }
};

class UsbDev
{
    enum{
        OUT_OF_USB_BUS = 128,
    };
private:
    int _isConnected;                               //is device connected
    int _deviceNumber;                              //device number on USB bus
    bool _nonSUdoDev;                               //device usable without sudo
    libusb_device           *_device;               //libusb device
    libusb_device_handle    *_device_handle;        //handle to USB device
    libusb_device_descriptor _device_descriptor;    //the device descriptor
    libusb_config_descriptor *_device_config;       //the config descriptor

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

    QStringList parseDeviceClass();

public:
    UsbDev();                                       //to create vector in container
    UsbDev(libusb_device *device, int devNr, QString *errorLog);
    ~UsbDev();

    virtual QString write(){
        return QString("Base class - can't write");
    }

    int deviceOpen();
    int deviceClose();

    int getNumOfPossibleConfigurations();
    QStringList getDeviceClass();
    int getVendorID();
    int getProductID();
    int getConfigDescriptor();

    bool isNonSudoDev() const{
        return _nonSUdoDev;
    }

    QString getProductString() const{
        return _product;
    }
    QString getManufacturerString() const{
        return _manufacturer;
    }

    QStringList devInfo();
    QStringList getConfigData();
};

class UsbHidDev : public UsbDev{
public:
   UsbHidDev(const UsbDev &parentUsbDev) : UsbDev(parentUsbDev) {}
   QString write();
};

#endif // USBDEV_H
