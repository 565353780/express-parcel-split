#ifndef Q3DSCENE_H
#define Q3DSCENE_H
#include <QObject>
#include <QOpenGLFunctions>
#include <QQuaternion>
#include <QVector3D>
#include <QMatrix4x4>
#include "../q3d_global.h"
namespace GCL {

/**
 * @brief The Q3DScene class
 *      一个简单的场景类， 用rotation/scale/offset来支持基本场景交互
 *
 *
 */
class QMesh3D;
class Q3DGCLSHARED_EXPORT Q3DScene : public QObject,protected QOpenGLFunctions
{
    Q_OBJECT
public:
    Q3DScene(QObject *parent = nullptr);

    void addModel(QMesh3D *model);

    void removeModel(QMesh3D *model);

public:
    virtual void init();

    virtual void render();

    virtual void resize(int w, int h);

    QMesh3D *getMesh(const QString &name);
public:
    void setProjectionParameters(qreal znear, qreal zfar,qreal fov);

    void setIdentity();
    virtual void lookat(const QVector3D& eye, const QVector3D& center, const QVector3D& up);

    virtual void manipulator_rotate(int dx, int dy);
    virtual void manipulator_scale(float sc);
    virtual void manipulator_move(int dx, int dy);
    void setManipulatorScaleMode(int mode) {manipulator_scale_mode_=mode;}

    bool getScreenRay(int x, int y, QVector3D &ray_point, QVector3D &ray_dir);

    QMatrix4x4 getDefaultModelMatrix() const;
    QMatrix4x4 getDefaultRotationMatrix() const;

    QMatrix4x4 getModelMatrix() const;
    QMatrix4x4 getProjectionMatrix() const;

    void setModelMatrix(const QMatrix4x4 &m);

    void setDefaultModelMatrix(const QMatrix4x4 &m);


    void setDefaultView();

    void setRotationSpeed(const qreal &rotation_speed);

    void setPanSpeed(const qreal &pan_speed);


    void setScaleSpeed(const qreal &sp);
    QVector3D project(const QVector3D &pos);
protected:

    QMatrix4x4 projection_matrix_;
    QMatrix4x4 model_matrix_;

    QMatrix4x4 default_model_matrix_;
    int width_{0};
    int height_{0};

    qreal zNear_{0.1};
    qreal zFar_{10000.0};
    qreal fov_{45.0};


    qreal rotation_speed_{1.0};
    qreal pan_speed_{1.0};
    qreal scale_speed_{1.0};

    int manipulator_scale_mode_{0};
};
}
#endif // Q3DSCENE_H
