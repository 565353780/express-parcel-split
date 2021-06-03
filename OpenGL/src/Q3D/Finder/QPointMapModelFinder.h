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
class QOffscreenSurface;
namespace GCL {
class QMesh3D;
class Q3DScene;
class QPointMapMesh;
class QShaderEvaluator;
class QAxesMesh;
class Q3DGCLSHARED_EXPORT QPointMapModelFinder : public QObject,protected QOpenGLFunctions
{
    Q_OBJECT
public:

    enum SymmetricType
    {
        NotSymmetry = 0,
        X_Axis_180 = 1,
        Y_Axis_180 = 1<<1,
        Z_Axis_180 = 1<<2,
        X_Axis_90 = 1<<3,
        Y_Axis_90 = 1<<4,
        Z_Axis_90 = 1<<5
    };
    struct ResultTransform{
      QVector3D    eular_;
      QVector3D    offset_;
      QVector2D     s_pos_;

      QMatrix4x4 getTransform() const;
      QQuaternion getQuat() const;
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


    void create();
    void destroy();


    void makeCurrent();
    void doneCurrent();

    void init();
    void resize(int w, int h);
    void render(); // for debug

    int width() const {return width_;}
    int height() const {return height_;}

    void setModelViewMatrix(const QMatrix4x4 &matrix);

    void clearResults();

    const QImage *getZMap() const { return &z_map_;}
public:
    void setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors);
    void loadPointMapPLY(const QString &filename, int w = 640, int h = 512, bool zflip = false, float flip_z = 0.0);
    void loadModel(const QString &filename);

    Q3DScene *getScene();
    QMesh3D *getMesh();
public:
    void updateBuffer();

    /**
     * @brief findModels
     * @param sample_points
     * 一般取前八个采样点作为种子点, 并为每个种子点迭代三轮;
     *
     */
    void findModels(const QList<QVector3D> &sample_points);

    void findModels();

    void quickFindAllModels();

    void findAtSeeds();

    qreal localSearching(QVector2D spos);

    void setSamplePoints( QList<QVector3D> &sample_points);
    void getSamplePoints( QList<QVector3D> &sample_points);

    void setDistanceTrunc(float trunc);

    void setDepthRange(float max_depth = 1000.0, float min_depth = 0.0);

    void setScoreThreshold(float score);

    void setPartLeastSize(int size) {u_part_least_size_ = size;}

    void setLocalPickPos(QVector3D o) {u_local_pickpos_ = o;}
    QVector3D getLocalPickPos() const {return u_local_pickpos_;}

    void setSymmetricType(int type) {symetry_type_ = type;}
    void setLocalIterationNum(int num) {u_local_iteration_num_ = num;}
    /**
     * @brief setExtendLevel
     * @param level
     * 边界延拓的级别
     */
    void setBoundaryExtendLevel(int level);


    void setDebugMode(bool t) {is_debug_mode_ = t;}
public:
    QList<ResultTransform> getResults() const;

private:
    QVector3D refineEulerIfSymmetry(QVector3D euler);
    SeedPoint getSeedBySamplePoint(const QVector3D &sp);

protected:
    float pack_range_{2048};
    float u_max_depth_{1000.0};
    float u_min_depth_{0.0};
    float u_mscore_{400.0};
    int u_part_least_size_{30};

    QVector3D u_local_pickpos_{0,0,18};
    QList<QVector3D> u_sample_points_;

    bool is_debug_mode_{false};
    float u_score_threshold_{1.0f};
    float u_trunc_{10};
    int u_boundary_level_{6};
    int u_local_iteration_num_{12};
    int width_{640};
    int height_{512};

    int u_group_size_{8};
    int u_scissor_level_{4};

    int symetry_type_{NotSymmetry};
protected:
    Q3DScene *scene_{nullptr};
    QPointMapMesh *pointmap_{nullptr};
    QMesh3D *mesh_{nullptr};
    QAxesMesh *axes_{nullptr};
    QShaderEvaluator *evaluator_{nullptr};

    QList<ResultTransform> results_;
    QList<SeedPoint> seeds_;
    QList<QVector3D> visited_points_;

protected:
    QOpenGLFramebufferObject *z_buffer_{nullptr};
    QImage z_map_;
    QOpenGLFramebufferObject *normal_buffer_{nullptr};

    QOpenGLFramebufferObject *tfbo_buffer_{nullptr};

    bool need_update_buffer_{true};


protected:
    QOpenGLContext *m_context_{nullptr};
    QOffscreenSurface *m_surface_{nullptr};
};
}
#endif // POINTMAPMODELFINDER_H
