#ifndef QAXESMESH_H
#define QAXESMESH_H
#include "Core/QMesh3D.h"
namespace GCL {
class QPrimitiveMesh;
class Q3DGCLSHARED_EXPORT QAxesMesh:public QMesh3D
{
public:
    QAxesMesh(QObject *parent = nullptr);


    void init() override;

    void render(const QMatrix4x4 &project_matrix, const QMatrix4x4 &model_matrix);


    void setRadius(float r) {radius_ = r;}
    void setLength(float l) {length_ = l;}
private:
    qreal radius_{2};
    qreal length_{100};
    QPrimitiveMesh *cylinder_{nullptr};
    QPrimitiveMesh *cone_{nullptr};
};
}
#endif // QAXESMESH_H
