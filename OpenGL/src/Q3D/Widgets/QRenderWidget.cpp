#include "QRenderWidget.h"
#include "../Core/Q3DScene.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <ctime>
#include <QPainter>
namespace GCL {

QRenderWidget::QRenderWidget(Q3DScene *scene, QWidget *parent):scene_(scene),QOpenGLWidget(parent)
{

}

QRenderWidget::QRenderWidget(QWidget *parent) : QOpenGLWidget(parent), scene_(new Q3DScene(this))
{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
//    format.setSamples(16);
    this->setFormat(format);
}

QRenderWidget::~QRenderWidget()
{
    if(scene_ && scene_->parent() == this)
    {
        makeCurrent();
        delete scene_;
        scene_ = nullptr;
        doneCurrent();
    }
}

void QRenderWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::RightButton)
    {
        lastPressdPos_ = event->pos();
    }
    else if(event->buttons() == Qt::MidButton)
    {
        lastPressdPos_ = event->pos();
    }
    else{
        QVector3D rp0;
        QVector3D rp1;
        scene_->getScreenRay(event->x(), this->height() - event->y(),rp0,rp1);

    }
}

void QRenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons()==Qt::RightButton)
    {
        QPoint dp = event->pos() - lastPressdPos_;
        scene_->manipulator_rotate(dp.x(),dp.y());
        lastPressdPos_ = event->pos();
        update();
    }
    else if(event->buttons() == Qt::MidButton)
    {
        QPoint dp = event->pos() - lastPressdPos_;
        scene_->manipulator_move(dp.x(),dp.y());
        lastPressdPos_ = event->pos();
        update();
    }
}

void QRenderWidget::mouseReleaseEvent(QMouseEvent *event)
{

}

void QRenderWidget::wheelEvent(QWheelEvent *e)
{
    if(e->delta() > 0)
    {
        scene_->manipulator_scale(1.1);
    }
    else
    {
        scene_->manipulator_scale(1.0/1.1);
    }
    update();
}

void QRenderWidget::initializeGL()
{
    this->makeCurrent();
    this->initializeOpenGLFunctions();
    scene_->init();
}

void QRenderWidget::resizeGL(int w, int h)
{
    scene_->resize(w,h);
}

void QRenderWidget::paintGL()
{
    clock_t t0 = clock();

    scene_->render();
//    qDebug()<<clock()-t0<<"ms";
}
}
