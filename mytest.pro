TEMPLATE = app

QT -= gui

CONFIG(release):DESTDIR = $$PWD/bin

CONFIG += c++15 console \
          release

CONFIG -= app_bundle \

DEFINES += QT_DEPRECATED_WARNINGS

DEFINES -= UNICODE

INCLUDEPATH = ./ie/include \
              ./ie/open_model_zoo_demos/common \
              ./ie/src/extension \
              ./ie/opencv/include

LIBS += -L"$$PWD/dll_backup/lib/" -lcpu_extension \
        -linference_engine \
        -lopencv_highgui412 \
        -lopencv_imgcodecs412 \
        -lopencv_imgproc412 \
        -lopencv_core412

SOURCES += main.cpp
