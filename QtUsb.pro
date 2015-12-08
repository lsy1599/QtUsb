#-------------------------------------------------
#
# Project created by QtCreator 2014-10-07T20:53:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtUsb
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        main-window.cpp \
    usbdev.cpp \
    usb-container.cpp \
    baseinfo.cpp

HEADERS  += main-window.h \
    usbdev.h \
    usb-container.h \
    baseinfo.h

FORMS    += mainwindow.ui

LIBS += -lusb-1.0
