#ifndef USB_HID_H 
#define USB_HID_H 

#include "usbdev.h"
#include <QString>

class UsbHid : public UsbDev {
private:

public:
    UsbHid();
    virtual ~UsbHid();
    QString control_transfer(uchar *data, size_t size, unsigned int request, size_t timeout=500);
};

#endif /* USB_HID_H */
