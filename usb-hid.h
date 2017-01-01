#ifndef USB_HID_H 
#define USB_HID_H 

#include "usbdev.h"
#include <QString>

class UsbHid : public UsbDev {
private:

public:
    UsbHid : public UsbDev();
    virtual ~UsbHid : public UsbDev();
    QString control_transfer(uchar *data, size_t size, unsigned int request, size_t timeout);
};

#endif /* USB_HID_H */
