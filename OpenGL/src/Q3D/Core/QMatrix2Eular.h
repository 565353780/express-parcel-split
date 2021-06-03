#ifndef QMATRIX2EULAR_H
#define QMATRIX2EULAR_H
#include <QMatrix4x4>
#include <QVector3D>
#include "q3d_global.h"
namespace GCL {
class Q3DGCLSHARED_EXPORT QMatrix2Euler
{
public:
    QMatrix2Euler();
    static QVector3D fromMatrixToEuler(const QMatrix4x4 &m, const QString &order = "ZYX");
};
}
#endif // QMATRIX2EULAR_H
