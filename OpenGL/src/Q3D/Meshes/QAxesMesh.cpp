#include "QAxesMesh.h"
#include "QPrimitiveMesh.h"
namespace GCL {

QAxesMesh::QAxesMesh(QObject *parent):QMesh3D(parent)
{

}

void QAxesMesh::init()
{
    QMesh3D::init();
    cylinder_ = new QPrimitiveMesh(QPrimitiveMesh::Cylinder,this);
    cone_ = new QPrimitiveMesh(QPrimitiveMesh::Cone,this);
//    cylinder_->setMaterial(this->getMaterial());
    cylinder_->init();
//    cone_->setMaterial(this->getMaterial());
    cone_->init();
}

void QAxesMesh::render(const QMatrix4x4 &project_matrix, const QMatrix4x4 &model_matrix)
{
    this->transform_matrix_.setToIdentity();
    this->transform_matrix_.translate(offset_);
    this->transform_matrix_.rotate(rotate_euler_[0],QVector3D(1,0,0));
    this->transform_matrix_.rotate(rotate_euler_[1],QVector3D(0,1,0));
    this->transform_matrix_.rotate(rotate_euler_[2],QVector3D(0,0,1));
    QMatrix4x4 scale_matrix;
    scale_matrix.scale(scale_);
    QMatrix4x4 _modelmatrix = model_matrix * this->transform_matrix_ * scale_matrix;

    float scale_radius = radius_ / 10.0;
    cylinder_->setColor(QVector3D(1.0,0.2,0.2));
    cylinder_->setEuler(QVector3D(0,90,0));
    cylinder_->setScale(QVector3D(scale_radius,scale_radius,length_ / 10.0));
    cylinder_->setOffset(cylinder_->localRotate(QVector3D(0,0,length_ / 2.0)));

    cone_->setColor(QVector3D(1.0,0.2,0.2));
    cone_->setEuler(QVector3D(0,90,0));
    cone_->setScale(QVector3D(scale_radius * 2.5,scale_radius * 2.5,scale_radius * 2.5));
    cone_->setOffset(cone_->localRotate(QVector3D(0,0,length_)));

    cylinder_->render(project_matrix,_modelmatrix);
    cone_->render(project_matrix,_modelmatrix);


    cylinder_->setColor(QVector3D(0.0,1.0,0.2));
    cylinder_->setEuler(QVector3D(90,0,0));
    cylinder_->setScale(QVector3D(scale_radius,scale_radius,length_ / 10.0));
    cylinder_->setOffset(cylinder_->localRotate(QVector3D(0,0,length_ / 2.0)));

    cone_->setColor(QVector3D(.2,1.0,0.2));
    cone_->setEuler(QVector3D(90,0,0));
    cone_->setScale(QVector3D(scale_radius * 2.5,scale_radius * 2.5,scale_radius * 2.5));
    cone_->setOffset(cone_->localRotate(QVector3D(0,0,length_)));

    cylinder_->render(project_matrix,_modelmatrix);
    cone_->render(project_matrix,_modelmatrix);

    cylinder_->setColor(QVector3D(0.2,0.2,1.0));
    cylinder_->setEuler(QVector3D(0,0,0));
    cylinder_->setScale(QVector3D(scale_radius,scale_radius,length_ / 10.0));
    cylinder_->setOffset(cylinder_->localRotate(QVector3D(0,0,length_ / 2.0)));

    cone_->setColor(QVector3D(0.2,0.2,1.0));
    cone_->setEuler(QVector3D(0,0,0));
    cone_->setScale(QVector3D(scale_radius * 2.5,scale_radius * 2.5,scale_radius * 2.5));
    cone_->setOffset(cone_->localRotate(QVector3D(0,0,length_)));

    cylinder_->render(project_matrix,_modelmatrix);
    cone_->render(project_matrix,_modelmatrix);



}
}
