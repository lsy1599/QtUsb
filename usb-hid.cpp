#include "usb-hid.h"

UsbHid::UsbHid() {}
UsbHid::~UsbHid() {}

QString UsbHid::control_transfer(uchar *data, size_t size, unsigned int request, size_t timeout) {
    libusb_control_transfer( getHandle(),
            LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
            request ,
            0,
            0,
            data, 
            size,
            timeout );
    return QString((char*)data);
}
