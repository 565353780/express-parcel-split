#ifndef Q3D_QMESH3D_H
#define Q3D_QMESH3D_H

#include <QObject>
#include <QOpenGLBuffer>
#include <QList>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QVector2D>
#include <QVector4D>
#include <QMatrix4x4>
#include "../q3d_global.h"
class QOpenGLShader;
class QOpenGLShaderProgram;
namespace GCL {
class QMaterial;
class Q3DGCLSHARED_EXPORT QMesh3D : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    struct VertexData
    {
        VertexData(const QVector3D &pos, const QVector3D &nor = QVector3D(), const QVector4D &color = QVector4D(),
                   const QVector2D &tc = QVector2D()):position_(pos),normal_(nor),color_(color),texCoord_(tc) {}
        VertexData(const QVector3D &pos, const QVector2D &tc):position_(pos),texCoord_(tc){}
        QVector3D position_;
        QVector3D normal_;
        QVector4D color_;
        QVector2D texCoord_;
    };

    using FaceData = uint;

    explicit QMesh3D(QObject *parent = nullptr);
    explicit QMesh3D(QMaterial *material, QObject *parent =nullptr);
    virtual ~QMesh3D();

    /**
     * @brief loadFile
     * @param filename
     * Support Obj only
     */
    virtual void loadFile(const QString &filename,bool flat = false);
    virtual void updateArrayBuffer(const std::vector<VertexData> &array_data);
    virtual void updateIndexBuffer(const std::vector<FaceData> &index_data,
                                   const uint &element_size = 0, const GLenum &type = GL_TRIANGLES);
    void updateIndexBuffer(const FaceData *data,   const uint &element_size, const GLenum &type = GL_TRIANGLES);
    QOpenGLShaderProgram *getProgram();
    QMaterial *getMaterial() {return material_;}
    virtual void setMaterial(QMaterial *program);
    virtual void init();
    virtual void render(const QMatrix4x4 &project_matrix = QMatrix4x4(),
                        const QMatrix4x4 &model_matrix = QMatrix4x4());

    bool isBackGround() const {return is_background_;}
    void setBackground(bool t) { is_background_ = t;}

    void setMatrixIdentidy();

    void rotate(const QQuaternion &quat);
    void rotate(float angle, const QVector3D &axis);

    void rotateEular(const QVector3D &eular);
    void translate(const QVector3D &offset);

    QVector3D getEular() const {return rotate_eular_;}
    QVector3D getOffset() const {return offset_;}

    void setEular(const QVector3D &e) {rotate_eular_ = e;}
    void setOffset(const QVector3D &o) {offset_ = o;}
    void initShader(const QString &vshader_filename, const QString &fshader_filename);

    QMatrix4x4 getTransform() const {return transform_matrix_;}

    int getVerticeSize() const {return verteice_size_;}
    QOpenGLBuffer *getVerticeArray() {return &array_buffer_;}
signals:

public slots:
protected:
    virtual void initDefaultShader();
protected:
    QString default_vshader_filename_{":/shaders/simple_vshader.glsl"};
    QString default_fshader_filename_{":/shaders/simple_fshader.glsl"};
    QMaterial *material_{nullptr};
    QOpenGLBuffer array_buffer_;
    QOpenGLBuffer index_buffer_{QOpenGLBuffer::IndexBuffer};
    QMatrix4x4 transform_matrix_;
    QVector3D rotate_eular_;
    QVector3D offset_;
    bool is_background_{false};
    int verteice_size_{0};
    int elements_size_{0};
    GLenum element_type_;
};

}
#endif // QMODEL3D_H
