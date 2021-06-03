#ifndef EASYMESH_H
#define EASYMESH_H

#include "../Q3D/Core/QMesh3D.h"
#include <QFileInfo>

using namespace GCL;

class EasyMesh : public QMesh3D
{
public:
    explicit EasyMesh(QObject *parent = nullptr);
    explicit EasyMesh(QMaterial *material, QObject *parent =nullptr);
    ~EasyMesh();

    virtual void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
    virtual void loadObjFile(const QString &filename, std::vector<QMesh3D::VertexData> &vertices, std::vector<QMesh3D::FaceData> &facedata, bool flat);
    virtual void loadFile(const QString &filename,bool flat = false);
    void updateTransformMatrix();

public:
    std::vector<VertexData> vertex_list;
    QMatrix4x4 transform_matrix;
};

#endif // EASYMESH_H
