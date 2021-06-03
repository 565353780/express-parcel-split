#ifndef POINTMAPWIDGET_H
#define POINTMAPWIDGET_H
#include "widgets/QRenderWidget.h"

namespace GCL {
class QPointMapMesh;
class Q3DScene;
class PointMapWidget : public QRenderWidget
{
    Q_OBJECT
public:
    PointMapWidget(QWidget *parent = nullptr);


    void initializeGL();
    void setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors);
private:
    QPointMapMesh *pointmap_{nullptr};
};

}
#endif // POINTMAPWIDGET_H
