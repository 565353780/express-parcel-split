#ifndef QRENDERWIDGET_H
#define QRENDERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "../q3d_global.h"
namespace GCL {
class Q3DScene;
class Q3DGCLSHARED_EXPORT QRenderWidget : public QOpenGLWidget,protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit QRenderWidget(Q3DScene *scene, QWidget *parent = nullptr);
    explicit QRenderWidget(QWidget *parent = nullptr);
    virtual ~QRenderWidget();
    Q3DScene *getScene() {return scene_;}
signals:

public slots:

public:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *e);
    void initializeGL();

protected:
    void resizeGL(int w, int h);
    void paintGL();

protected:
    Q3DScene *scene_{nullptr};


private:
    QPoint lastPressdPos_;
};
}
#endif // QRENDERWIDGET_H
