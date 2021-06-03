#ifndef QCUBEMESH_H
#define QCUBEMESH_H
#include "../Core/QMesh3D.h"
namespace GCL {

class Q3DGCLSHARED_EXPORT QCubeMesh : public QMesh3D
{
public:
    explicit QCubeMesh(QObject *parent = nullptr);
    explicit QCubeMesh(QMaterial *material, QObject *parent = nullptr);

    void init() override;
};
}
#endif // CUBEMODEL_H
