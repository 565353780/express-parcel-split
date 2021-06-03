QT += core
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app

DESTDIR = ../bin

DEFINES += QT_DEPRECATED_WARNINGS

win32{
DEFINES += WIN32
}
unix{
DEFINES += Linux
}

SOURCES += main.cpp

HEADERS += ../include/darknet.h

unix{
DEFINES += GPU CUDNN

LIBS += ../include/libdarknet.so \
        /usr/local/cuda-9.0/lib64/libcudart.so.9.0 \
        /usr/local/cuda-9.0/lib64/libcudnn.so.7 \
        /usr/local/cuda-9.0/lib64/libcurand.so.9.0 \
        /usr/local/cuda-9.0/lib64/libcublas.so.9.0

INCLUDEPATH += /usr/local/cuda-9.0/targets/x86_64-linux/include
}
