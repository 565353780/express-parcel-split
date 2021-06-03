QT +=core gui widgets opengl
TEMPLATE = lib

DESTDIR = ../../lib

TARGET = Q3d_gcl
RESOURCES += \
    q3d.qrc

DISTFILES +=

DEFINES += Q3D_GCL_LIBRARY

HEADERS += \
    q3d_global.h \
    Meshes/QCubeMesh.h \
    Meshes/QPointMapMesh.h \
    Meshes/QQuadMesh.h \
    Widgets/QRenderWidget.h \
    Core/Q3DScene.h \
    Core/QMaterial.h \
    Core/QMesh3D.h \
    Core/QMeshParser.h \
    Widgets/QModelFinderWidget.h \
    Evaluator/QShaderEvaluator.h \
    Finder/QPointMapModelFinder.h

SOURCES += \
    Meshes/QCubeMesh.cpp \
    Meshes/QPointMapMesh.cpp \
    Meshes/QQuadMesh.cpp \
    Widgets/QRenderWidget.cpp \
    Core/Q3DScene.cpp \
    Core/QMaterial.cpp \
    Core/QMesh3D.cpp \
    Core/QMeshParser.cpp \
    Widgets/QModelFinderWidget.cpp \
    Evaluator/QShaderEvaluator.cpp \
    Finder/QPointMapModelFinder.cpp





#win32: LIBS += -L$$PWD/../../tifflib/ -llibtiff

#INCLUDEPATH += $$PWD/../../tifflib/inc
#DEPENDPATH += $$PWD/../../tifflib
