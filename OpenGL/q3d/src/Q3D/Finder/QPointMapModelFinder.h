#ifndef QPOINTMAPMODELFINDER_H
#define QPOINTMAPMODELFINDER_H
#include "../q3d_global.h"
#include <QObject>
#include <QOpenGLFunctions>
#include <QList>
#include <QQuaternion>
#include <QVector3D>
#include <QImage>
class QOpenGLFramebufferObject;

namespace GCL {
class QMesh3D;
class Q3DScene;
class QPointMapMesh;
class QShaderEvaluator;
class Q3DGCLSHARED_EXPORT QPointMapModelFinder : public QObject,protected QOpenGLFunctions
{
    Q_OBJECT
public:
    struct ResultTransform{
      QVector3D    eular_;
      QVector3D    offset_;
      qreal score_{0.0};
    };

    struct SeedPoint{
      QVector2D s_pos_;  // screen position (0.0 - 1.0,0.0 - 1.0)
      QVector3D o_pos_;  // origin position
      QVector3D eular_;
      QVector3D offset_;
      qreal score_{0.0};
    };

    QPointMapModelFinder(Q3DScene *scene = nullptr,QObject *parent = nullptr);
    ~QPointMapModelFinder();
    void init();
    void resize(int w, int h);
    void render(); // for debug

    int width() const {return width_;}
    int height() const {return height_;}

    void setModelViewMatrix(const QMatrix4x4 &matrix);


public:
    void setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors);
    void loadPointMapPLY(const QString &filename, int w = 640, int h = 512, bool zflip = false, float flip_z = 0.0);
    void loadModel(const QString &filename);

    Q3DScene *getScene();
    QMesh3D *getMesh();
public:
    void updateBuffer();

    void findModels();

    void findBySeeds();


    void getSamplePoints( QList<QVector3D> &sample_points);

    void setDistanceTrunc(float trunc);

    void setDepthRange(float max_depth = 1000.0, float min_depth = 0.0);

    void setScoreLevel(float score);

    void setPartLeastSize(int size) {u_part_least_size_ = size;}

    /**
     * @brief setExtendLevel
     * @param level
     * 边界延拓的级别
     */
    void setBoundaryExtendLevel(int level);
public:
    QList<ResultTransform> results() const;


protected:
    float pack_range_{2048};
    float u_max_depth_{1000.0};
    float u_min_depth_{0.0};
    float u_mscore_{400.0};
    int u_part_least_size_{30};

    float u_score_level_{1.3f};
    float u_trunc_{8};
    int u_boundary_level_{6};
    int width_{640};
    int height_{512};
protected:
    Q3DScene *scene_{nullptr};
    QPointMapMesh *pointmap_{nullptr};
    QMesh3D *mesh_{nullptr};
    QShaderEvaluator *evaluator_{nullptr};

    QList<ResultTransform> results_;
    QList<SeedPoint> seeds_;



protected:
    QOpenGLFramebufferObject *z_buffer_{nullptr};
    QImage z_map_;
    QOpenGLFramebufferObject *normal_buffer_{nullptr};

    bool need_update_buffer_{true};
};
}
#endif // POINTMAPMODELFINDER_H
