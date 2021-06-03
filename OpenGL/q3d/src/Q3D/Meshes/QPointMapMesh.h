#ifndef QPOINTMAPMESH_H
#define QPOINTMAPMESH_H
#include "../Core/QMesh3D.h"
class QOpenGLFramebufferObject;
namespace GCL {
class QMaterial;
class QQuadMesh;
class Q3DGCLSHARED_EXPORT QPointMapMesh : public QMesh3D
{
public:
    enum MapType
    {
        Z_Map = 0,
        Normal_Map = 1,
        X_FloatMap = 2,
        Y_FloatMap = 3,
        Z_FloatMap = 4
    };
    explicit QPointMapMesh(QObject *parent = nullptr);
    explicit QPointMapMesh(QMaterial *material, QObject *parent =nullptr);

    void init() override;
    void setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors);

    void render(const QMatrix4x4 &project_matrix = QMatrix4x4(),
                            const QMatrix4x4 &model_matrix = QMatrix4x4()) override;
    void updateNormalImage(const QImage &x_map, const QImage &y_map, const QImage &z_map, QImage &nor_map);


    void loadPLY(const QString &filename, int w, int h, bool zflip = false, float flip_z = 0.0);

//    void loadTiff(const QString &filename);
    void renderMap(QOpenGLFramebufferObject *fbo, int mapType, const QMatrix4x4 &project_matrix = QMatrix4x4(),
                   const QMatrix4x4 &model_matrix = QMatrix4x4());
protected:
    void renderFBO(QOpenGLFramebufferObject *fbo, QMaterial *material, QMesh3D *mesh, const QMatrix4x4 &matrix = QMatrix4x4());
protected:
    int pointmap_w_{1};
    int pointmap_h_{1};
    QQuadMesh *quad_mesh_{nullptr};
};
}
#endif // QPOINTMAPMESH_H
