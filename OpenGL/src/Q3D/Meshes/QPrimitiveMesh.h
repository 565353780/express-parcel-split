#ifndef QPRIMITIVEMESH_H
#define QPRIMITIVEMESH_H
#include "Core/QMesh3D.h"
namespace GCL {
class Q3DGCLSHARED_EXPORT QPrimitiveMesh:public QMesh3D
{
    Q_OBJECT
public:
    enum PrimitiveType
    {
        Sphere = 0,
        Cylinder = 1,
        Cone = 2
    };
    QPrimitiveMesh(PrimitiveType type, QObject *parent = nullptr);

    void init();

protected:
private:
    PrimitiveType primitive_type_{Sphere};

};
}
#endif // QPRIMITIVEMESH_H
