#ifndef QSHMATERIAL_H
#define QMATERIAL_H
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMap>
#include <memory>
#include "../q3d_global.h"
namespace GCL {
class Q3DGCLSHARED_EXPORT QMaterial : public QOpenGLShaderProgram, protected QOpenGLFunctions
{

public:
    explicit QMaterial(QObject *parent = 0);
    virtual ~QMaterial();

    void clearTextures();
    void linkShaders(const QString& vert_filename, const QString &frag_filename);

    void addUniformValue(const char *name, GLfloat f);
    void addUniformValue(const char *name, GLint val);
    void addUniformValue(const char *name ,QVector2D v);
    void addUniformValue(const char *name, QVector3D v);
    void addUniformValue(const char *name, QVector4D v);
    void addUniformTexture(const char *name, GLuint tex_id, bool add_to_local = false);
    GLuint addUniformTextureImage(const char *name, const QImage &image,
                                  QOpenGLTexture::Filter minfilter = QOpenGLTexture::Nearest,
                                  QOpenGLTexture::Filter magfilter = QOpenGLTexture::Linear,
                                  QOpenGLTexture::WrapMode warpmode = QOpenGLTexture::Repeat
                                  );



    static QVector4D packInt(uint val);
    static uint unpackInt(QVector4D v);
    static QVector4D pack(double val);
    static double unpack(QVector4D v);
    virtual void setUniforms(int &active_texture_count);
protected:
    struct UniformType
    {
        enum {
           Float = 0,
           Int = 1,
           Texture =2,
           Vec2=  3,
           Vec3 = 4,
           Vec4 = 5

        };
        GLint i_val_;
        GLfloat f_val_;
        GLuint tex_id_;
        QVector4D v_val_;
        int type_{Float};
    };
    QMap<QString, UniformType> uniform_map_;

    QList< std::shared_ptr<QOpenGLTexture> > local_textures_;
    QList< uint > local_texture_ids_;
};
}
#endif // QMATERIAL_H
