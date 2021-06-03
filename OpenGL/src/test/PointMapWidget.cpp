#include "PointMapWidget.h"
#include "Core/Q3DScene.h"
#include "Meshes/QPointMapMesh.h"
#include <QSurfaceFormat>

#include <QOpenGLContext>
#include <QDebug>
namespace GCL {

PointMapWidget::PointMapWidget(QWidget *parent):QRenderWidget(parent)
{
    scene_->setRotationSpeed(2);
    scene_->setPanSpeed(40);
    scene_->setScaleSpeed(1);
    scene_->setProjectionParameters(0.1,10000.0,45);
    this->show();
}

void PointMapWidget::initializeGL()
{
    QRenderWidget::initializeGL();
    pointmap_ = new QPointMapMesh(getScene());

}

void PointMapWidget::setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors)
{
    pointmap_->setPointMap(w,h,points,colors);
}

}
