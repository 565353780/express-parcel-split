#ifndef QQUADMESH_H
#define QQUADMESH_H
#include "../Core/QMesh3D.h"
namespace GCL {

class Q3DGCLSHARED_EXPORT QQuadMesh : public QMesh3D
{
public:
public:
     QQuadMesh(QObject *parent = nullptr);
     QQuadMesh(QMaterial *material, QObject *parent = nullptr);
    void init() override;
};
}
#endif // QQUATMESH_H
