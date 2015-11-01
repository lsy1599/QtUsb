#ifndef USBDEV_H
#define USBDEV_H

#include <libusb-1.0/libusb.h>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <QTextStream>
#include <QVector>


// TODO change libusb struct

namespace USBParser{
    QString parseDeviceClass(int code);
    QString parseUsbError(int error);
}

// It was struct - but to avoid warnings considering default 0xFF values which are far off
// Values which could be gained during usage (f.e. max adress - 127) constructor had to be
// created
class Endpoint : public libusb_endpoint_descriptor {
private:
    QString extra_;
public:
    enum Direction {
        In  = 0,
        Out = 1,
    };
    Endpoint();
    Endpoint(const libusb_endpoint_descriptor *endpoint);
    unsigned char getbEndpointAddress();
    unsigned int decriptorSize();
    QString direction();
    unsigned int endpointNr();
    unsigned int maxPacketSize();
    unsigned int pollintInterval();
    unsigned int refreshFeedback();
    unsigned int synchAddress();
    QStringList parse_bmAttributes();
    bool exist() const;
};

class AlternateSetting {
private:
    libusb_interface_descriptor interface_descriptor;
    QVector<Endpoint> _endpoint;
    QString _extra;
public:
    AlternateSetting();
    AlternateSetting(const libusb_interface_descriptor* toGet);
    ~AlternateSetting();
    bool exist();
    int interaceClass();
    Endpoint getEndpoint(int nr) const;
    Endpoint getEndpoint(Endpoint::Direction IO);
    bool setEndpoint(Endpoint toPush);
    QString InterfaceProtocol();
    int getInterfaceNr();
//    size_t getEndpointNum();
};

// TODO return whole information about all interfaces accessible

class Interface{
private:
    int _numOfAltInterfaces;
    QVector<AlternateSetting> _alternateSetting;

public:
    Interface();
    Interface(int numOfAltInterfaces);
    Interface(const libusb_interface* toGet);
    QStringList getInterfacesNames();
    AlternateSetting fetchInterface(QString name);
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

    int open();
    int close();
    int getNumOfPossibleConfigurations();
    int getVendorID();
    int getProductID();
    QStringList getDeviceClass();
    int getConfigDescriptor();

    libusb_device_handle *getHandle();
    // return Interface of that type
    AlternateSetting getInterface(QString endpointType);
    // returns first endpoint address of that type
    unsigned char getEndpoint(QString endpointType, QString &result);
    bool isNonSudoDev() const;
    QString getProductString() const;
    QString getManufacturerString() const;
    QStringList devInfo();
    QStringList getConfigData();
    QString write();

private:
    QStringList parseDeviceClass();

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

};

class UsbHidDev : public UsbDev{
public:
   UsbHidDev(const UsbDev &parentUsbDev) : UsbDev(parentUsbDev) {}
   QString write();
};

#endif // USBDEV_H
