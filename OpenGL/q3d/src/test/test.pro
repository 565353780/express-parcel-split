QT +=core gui widgets opengl

DESTDIR = ../../bin
SOURCES += \
    main.cpp \
    PointMapWidget.cpp \
    easymesh.cpp

win32: LIBS += -L$$PWD/../../lib/ -lQ3d_gcl

INCLUDEPATH += $$PWD/../../ \
               $$PWD/../Q3D
DEPENDPATH += $$PWD/../../

HEADERS += \
    PointMapWidget.h \
    easymesh.h

LIBS += -lopengl32 -lglu32
