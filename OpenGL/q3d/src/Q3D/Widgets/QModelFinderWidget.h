#ifndef QMODELFINDERWIDGET_H
#define QMODELFINDERWIDGET_H

#include "QRenderWidget.h"
#include <QVector3D>
#include <QVector2D>
class QOpenGLFramebufferObject;
namespace GCL {
class QPointMapMesh;
class QMesh3D;
class QShaderEvaluator;
class Q3DGCLSHARED_EXPORT QModelFinderWidget : public QRenderWidget
{
public:
    struct ResultTransform
    {
        QVector3D eular_;
        QVector3D offset_;
        qreal score_;
    };

    struct SeedPoint{
      QVector2D s_pos_;  // screen position (0.0 - 1.0,0.0 - 1.0)
      QVector3D o_pos_;  // origin position
      QVector3D eular_;
      QVector3D offset_;
      qreal score_{0.0};
    };


    QModelFinderWidget(QWidget *parent = nullptr);
    ~QModelFinderWidget();

    void setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors);
    void loadPointMapPLY(const QString &filename, int w = 1024, int h = 819, bool zflip = false, float flip_z = 0.0);
    void loadModel(const QString &filename);

    void setScoreLevel(const qreal &score_level);
    void setDepthRange(float max_depth = 1000.0, float min_depth = 0.0);
    void setBoundaryExtendLevel(int level);
    void setPartLeastSize(int size);
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);



protected:
    void findAllModel();

    void findAllModelBySeeds();
    void findBySeeds();
    void getSamplePoints( QList<QVector3D> &sample_points);
    double testFindModel();

    double findModel();

    double findModel2();

    double findModel3();

    void printArray();

    void getArray(QList<double> &res);
    QOpenGLFramebufferObject *z_buffer_{nullptr};
    QOpenGLFramebufferObject *normal_buffer_{nullptr};
    QOpenGLFramebufferObject *fbo_{nullptr};

    QOpenGLFramebufferObject *array_fbo_{nullptr};


    bool need_update_buffer_{false};
    QShaderEvaluator *evaluator_{nullptr};
    QPointMapMesh *pointmap_{nullptr};
    QMesh3D *mesh_{nullptr};
    int u_boundary_level_{6};
    int u_part_least_size_{30};

    QImage z_map_;
    QImage f_map0_;
    QImage f_map1_;
    float u_max_depth_{1000.0};
    float u_min_depth_{0.0};
    qreal score_level_{1.2};
    QList< ResultTransform > results_;

    QList< SeedPoint > seeds_;

};
}
#endif // QMODELFINDERWIDGET_H
