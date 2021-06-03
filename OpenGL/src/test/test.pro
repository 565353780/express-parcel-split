QT +=core gui widgets opengl
DESTDIR = ../../bin
SOURCES += \
    main.cpp \
    PointMapWidget.cpp

win32: LIBS += -L$$PWD/../../lib/ -lq3d_gcl

INCLUDEPATH += $$PWD/../../ \
               $$PWD/../Q3D
DEPENDPATH += $$PWD/../../

HEADERS += \
    PointMapWidget.h
