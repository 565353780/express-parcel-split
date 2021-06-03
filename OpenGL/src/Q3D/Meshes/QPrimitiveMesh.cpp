#include "QPrimitiveMesh.h"
#include "Core/QMaterial.h"

namespace GCL {

QPrimitiveMesh::QPrimitiveMesh(PrimitiveType type, QObject *parent):QMesh3D(parent),primitive_type_(type)
{

}

void QPrimitiveMesh::init()
{
    QMesh3D::init();

    if(Sphere == primitive_type_)
    {
        this->loadFile(":/geometries/ball.obj");
    }
    else if(Cylinder == primitive_type_)
    {
        this->loadFile(":/geometries/cylinder.obj",true);
    }
    else if(Cone == primitive_type_)
    {
        this->loadFile(":/geometries/cone.obj",true);
    }

}

}
