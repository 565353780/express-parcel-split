#include "Q3DScene.h"
#include "QMesh3D.h"
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QDebug>
#include "QMaterial.h"
#include <QRegularExpression>
namespace GCL {


Q3DScene::Q3DScene(QObject *parent):QObject(parent)
{

}

void Q3DScene::init()
{
//    qDebug()<<"Init Q3DScene";
    this->initializeOpenGLFunctions();
//    qDebug()<<"Init Q3DScene";

    this->model_matrix_.setToIdentity();

    this->model_matrix_.translate(QVector3D(0.0, 0.0, -1.0));

    this->projection_matrix_.setToIdentity();

    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//    glEnable(GL_DOUBLES);


}

void Q3DScene::addModel(QMesh3D *model)
{
    model->setParent(this);
}

void Q3DScene::removeModel(QMesh3D *model)
{
    model->setParent(nullptr);
    model->deleteLater();
}

void Q3DScene::setProjectionParameters(qreal znear, qreal zfar, qreal fov)
{
    zNear_ = znear;
    zFar_ = zfar;
    fov_ = fov;
}

void Q3DScene::setIdentity()
{
    model_matrix_.setToIdentity();
    projection_matrix_.setToIdentity();
}

void Q3DScene::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );


//    QMatrix4x4 matrix = this->projection_matrix_ * this->model_matrix_;
    QRegularExpression re;

    QList<QMesh3D*> models = this->findChildren<QMesh3D*>(re,Qt::FindDirectChildrenOnly);
    foreach (QMesh3D *model, models) {
        if(!model->isVisible()) continue;
        model->render(this->projection_matrix_,this->model_matrix_);
    }
    glDisable(GL_DEPTH_TEST);
}

void Q3DScene::resize(int w, int h)
{
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Reset projection
    projection_matrix_.setToIdentity();

    // Set perspective projection
    projection_matrix_.perspective(fov_, aspect, zNear_, zFar_);

    glViewport(0,0,w,h);
    width_ = w;
    height_ = h;

    //    qDebug()<<this->model_matrix_<<" "<<this->projection_matrix_;
}

QMesh3D *Q3DScene::getMesh(const QString &name)
{
    QMesh3D *mesh = findChild<QMesh3D *>(name);
    return mesh;
}

void Q3DScene::lookat(const QVector3D &eye, const QVector3D &center, const QVector3D &up)
{
    model_matrix_.lookAt(eye.normalized(),center.normalized(),up);
    model_matrix_.translate(QVector3D(0,0,-center.length() * 0.01));
//    model_matrix_.scale((eye-center).length());
}

void Q3DScene::manipulator_rotate(int dx, int dy)
{
    QVector3D axis = QVector3D(-dy, dx,0);

    bool invertable = false;
    QMatrix4x4 invertM = model_matrix_.inverted(&invertable);
    qreal length = axis.length();
    axis.normalize();

    if(invertable)
    {
        QVector4D leftDir = invertM.map(QVector4D(1,0,0,0));
        QVector4D upDir = invertM.map(QVector4D(0,1,0,0));
        QVector4D eye = invertM.map(QVector4D(0,0,1,0));

        QVector3D lv(leftDir.x(),leftDir.y(),leftDir.z());
        QVector3D uv(upDir.x(), upDir.y(), upDir.z());
        QVector3D zv(eye.x(),eye.y(),eye.z());
        QVector3D mv = dx * lv - dy * uv;
        mv.normalize();

        axis = QVector3D::crossProduct(zv,mv);
        axis.normalize();
    }

    model_matrix_.rotate(length*0.1*rotation_speed_,axis);
}

void Q3DScene::manipulator_scale(float sc)
{

    if(manipulator_scale_mode_ == 1)
    {
        model_matrix_.translate(0,0, (sc-1.0) *100.0 * scale_speed_ );
    }
    else
    {
        model_matrix_.scale(sc);
    }
}

void Q3DScene::manipulator_move(int dx, int dy)
{
    QVector3D axis = QVector3D(dx, -dy,0);

    bool invertable = false;
    QMatrix4x4 invertM = model_matrix_.inverted(&invertable);
    qreal length = axis.length();
    axis.normalize();
    if(invertable)
    {
        QVector4D leftDir = invertM.map(QVector4D(1,0,0,0));
        QVector4D upDir = invertM.map(QVector4D(0,1,0,0));
        QVector4D eye = invertM.map(QVector4D(0,0,1,0));

        QVector3D lv(leftDir.x(),leftDir.y(),leftDir.z());
        QVector3D uv(upDir.x(), upDir.y(), upDir.z());
        QVector3D zv(eye.x(),eye.y(),eye.z());
        QVector3D mv = dx * lv - dy * uv;
        axis = mv;
        axis.normalize();
    }

    model_matrix_.translate(axis*length*0.01 * pan_speed_);
}

bool Q3DScene::getScreenRay(int x, int y, QVector3D &ray_point, QVector3D &ray_dir)
{
    QMatrix4x4 matrix = projection_matrix_ * model_matrix_;
    bool invertable = false;
    matrix = matrix.inverted(&invertable);
    if(invertable)
    {
        QVector3D v0((x/(qreal)width_)*2.0- 1.0,(y/(qreal)height_)*2.0 - 1.0,0.99);
        QVector3D v1 = v0 - QVector3D(0,0,0.01);

        v0 = matrix.map(v0);
        v1 = matrix.map(v1);
        ray_point = v0;
        ray_dir = v1 - v0;
        ray_dir.normalize();
        return true;

    }
    else
    {
        return false;
    }
}

QMatrix4x4 Q3DScene::getDefaultModelMatrix() const
{
    return default_model_matrix_;
}

QMatrix4x4 Q3DScene::getDefaultRotationMatrix() const
{
    QVector3D offset = default_model_matrix_.map(QVector3D(0,0,0));
    QMatrix4x4 m;
    m.translate(-offset);
    m = m * default_model_matrix_;

    return m;
}

QMatrix4x4 Q3DScene::getModelMatrix() const
{
    return model_matrix_;
}

QMatrix4x4 Q3DScene::getProjectionMatrix() const
{
    return projection_matrix_;
}

void Q3DScene::setModelMatrix(const QMatrix4x4 &m)
{
    model_matrix_ = m;
}

void Q3DScene::setDefaultModelMatrix(const QMatrix4x4 &m)
{
    default_model_matrix_ = m;
}

void Q3DScene::setDefaultView()
{
    model_matrix_ = default_model_matrix_;
}


void Q3DScene::setRotationSpeed(const qreal &rotation_speed)
{
    rotation_speed_ = rotation_speed;
}

void Q3DScene::setPanSpeed(const qreal &pan_speed)
{
    pan_speed_ = pan_speed;
}

void Q3DScene::setScaleSpeed(const qreal &sp)
{
    scale_speed_ = sp;
}

QVector3D Q3DScene::project(const QVector3D &pos)
{
    return (projection_matrix_ * model_matrix_).map(pos);
}

}
