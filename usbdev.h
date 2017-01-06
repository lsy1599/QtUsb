#ifndef USBDEV_H
#define USBDEV_H

#include <libusb-1.0/libusb.h>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <QTextStream>
#include <QVector>
#include <QList>
#include "baseinfo.h"
#include <QDebug>


// TODO change libusb struct

namespace USBParser
{
QString parseDeviceClass(int code);
QString parseUsbError(int error);
}

// It was struct - but to avoid warnings considering default 0xFF values which are far off
// Values which could be gained during usage (f.e. max adress - 127) constructor had to be
// created
class Endpoint : public libusb_endpoint_descriptor
{
private:
    QString extra_;
public:
    enum Direction {
        In  = 0x80,
        Out = 0,
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

class AlternateSetting
{
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
    Endpoint getEndpoint(int nr) throw(Error);
    Endpoint getEndpoint(Endpoint::Direction IO);
    bool setEndpoint(Endpoint toPush);
    QString InterfaceProtocol();
    int getInterfaceNr();
//    size_t getEndpointNum();
};

// TODO return whole information about all interfaces accessible

class Interface
{
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

// AND add while (pop != nop ) -> corr action ( in catch )
// fun cleanState
// AND add state to dev.
// ADD new state use to get all devices and so one
class UsbDev
{
    enum {
        OUT_OF_USB_BUS = 128,
    };

public:
    UsbDev();                                       //to create vector in container
    UsbDev(libusb_device *device, int devNr, QString *errorLog);
    ~UsbDev();

    class State
    {
    private:
        QVector<char> interfacesNrClaimed;
    public:
        enum ST {
            None = 0,
            Opened,
            Claimed,
        };
        ST pop()
        {
            if ( state.isEmpty() ) {
                return None;
            } else {
                ST tmp = state.back();
                state.pop_back();
                return tmp;
            }
        }
        void push(State::ST st)
        {
            state.push_back(st);
        }

        void push(char interfaceNr)
        {
            state.push_back(State::ST::Claimed);
            interfacesNrClaimed.push_back(interfaceNr);
        }

        char popInNr()
        {
            char tmp = interfacesNrClaimed.back();
            interfacesNrClaimed.pop_back();
            return tmp;
        }

        State() { }
    private:
        QList<ST> state;
    };

    State st;
    int open() throw(Error);
    int close();
    // Usb configs
    int getNumOfPossibleConfigurations();
    int getVendorID();
    int getProductID();
    QStringList getDeviceClass();
    int getConfigDescriptor();
    // Usb connection elements
    libusb_device_handle *getHandle() throw(Error);
    AlternateSetting getInterface(QString endpointType) throw(Error);                  // return Interface of that type
    unsigned char getEndpoint(QString endpointType, QString &result);     // returns first endpoint address of that type
    bool isNonSudoDev() const;
    // Usb configs cd
    QString getProductString() const;
    QString getManufacturerString() const;
    QStringList devInfo();
    QStringList getConfigData();
    // TODO maybe I shall add template to fit it into "SIMPLE" mode when I do not need to think
    // about reconnection of the device, instead of doing it by default
    QString interrupt_transfer(Endpoint::Direction IO, const unsigned int timeout=5000 );
    QString control_transfer(Endpoint::Direction IO, unsigned int request, uchar* bufer, size_t buf_size, const unsigned int timeout=5000 );
    QString control_transfer(uchar *data, size_t size, unsigned int request, size_t timeout=500);
//    QString pollRead();
    void deClaim() throw(Error);
    bool freeStates();

private:
    QStringList parseDeviceClass();
    void checkAndDetachKernelDriver(libusb_device_handle* handle, uint8_t interfaceNr );
    void checkBuffor( size_t buffSize, size_t eBuffSize, Endpoint::Direction IO ) throw(Error);
    void claim_interface(libusb_device_handle* handle, unsigned int epNr ) throw(Error);

    int _isOpen;                                    //is device connected
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

    bool _isUserConnectable;
    QStringList _deviceLog;

};

#endif
