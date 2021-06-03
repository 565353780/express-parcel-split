#ifndef QMESHPARSER_H
#define QMESHPARSER_H
#include "QMesh3D.h"
namespace GCL {

class Q3DGCLSHARED_EXPORT QTriMeshParser
{
public:
    QTriMeshParser();


    static void loadFile(const QString &filename,
                         std::vector<QMesh3D::VertexData> &vertices,
                         std::vector<QMesh3D::FaceData> &facedata,
                         bool flat = false
                         );


    static void loadObjFile(const QString &filename,
                            std::vector<QMesh3D::VertexData> &vertices,
                            std::vector<QMesh3D::FaceData> &facedata, bool flat = false);

};
}
#endif // QMESHPARSER_H
