#include "QQuadMesh.h"

namespace GCL {

QQuadMesh::QQuadMesh(QObject *parent):QMesh3D(parent)
{
    this->init();
}

QQuadMesh::QQuadMesh(QMaterial *material, QObject *parent):QMesh3D(material,parent)
{
    this->init();
}

void QQuadMesh::init()
{
    QMesh3D::init();
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-1.0f, -1.0f,  0.0f), QVector2D(0.0f,1.0f)},  // v0
        {QVector3D( 1.0f, -1.0f,  0.0f), QVector2D(1.f, 1.0f)}, // v1
        {QVector3D( 1.0f,  1.0f,  0.0f), QVector2D(1.f, 0.f)}, // v2
        {QVector3D(-1.0f,  1.0f,  0.0f), QVector2D(0.0f, 0.f)}  // v3
    };

    std::vector<VertexData> datalist;
    for(int i=0; i < 4; i++)
    {
        datalist.push_back(vertices[i]);
    }
    FaceData indices[] = {
         0,  1,  2,  0, 2, 3    // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
    };


    this->updateArrayBuffer(datalist);
    this->updateIndexBuffer(indices,6,GL_TRIANGLES);
}

void QQuadMesh::updateUniforms()
{

}

}
