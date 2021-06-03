#ifndef QSHADEREVALUATOR_H
#define QSHADEREVALUATOR_H
#include <QObject>
#include <QOpenGLFunctions>
#include <QImage>
#include "../q3d_global.h"
class QOpenGLFramebufferObject;
namespace GCL {
class QMaterial;
class QMesh3D;
class Q3DGCLSHARED_EXPORT QShaderEvaluator : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    enum MapSize
    {
        Size_1 = 1,
        Size_2 = 2,
        Size_4 = 4,
        Size_8 = 8,
        Size_16 = 16,
        Size_32 = 32,
        Size_64 = 64,
        Size_128 = 128,
        Size_256 = 256,
        Size_512 = 512,
        Size_1024 = 1024,
        Size_2048 = 2048,
        Size_4096 = 4096
    };
    enum ValueType
    {
        VT_Red = 0,
        VT_Pack = 1
    };
    enum FuncType
    {
        Func_Sum = 0,
        Func_Max = 1,
        Func_Count = 2
    };
    struct Problem
    {
        int w_;
        int h_;
        int val_type_{VT_Red};
        int func_type_{Func_Max};
        float valid_min_{0.0};
        float valid_max_{1.0};
        GLuint texture_id_;
        int map_size_{Size_1024};
    };
    QShaderEvaluator(QObject *parent= nullptr);
    ~QShaderEvaluator();


    void updateArrayValue(int x, int y);
    void initArray(int w, int h);
    double countTexture(GLuint texture_id,int w, int h, double vmin, double vmax, MapSize map_size = Size_1024);
    double getSumTexture(GLuint texture_id,int w, int h,MapSize map_size = Size_1024);
    double evalTexture(const Problem& problem);

    double evalTexture(GLuint texture_id, int val_type, int func_type, int w, int h, MapSize map_size = Size_1024);
    qreal getPackRange() const;
    void setPackRange(const qreal &r);

//    QImage image_;

    GLuint getResultTextureId();
    QOpenGLFramebufferObject *getResultFrameBufferObj();

    QOpenGLFramebufferObject *getFrameBufferObj(MapSize s);

    void setToImage(bool t);
protected:
    void renderFBO(QOpenGLFramebufferObject *fbo, QMaterial *material, QMesh3D *mesh);

    void initMaterial();
protected:
    QMaterial *material_{nullptr};
    qreal pack_range_{256*256*256};
    QMap<int, QOpenGLFramebufferObject *> fbos_;

    int current_array_fbo_{0};
    int array_width_{0};
    int array_height_{1};
    bool need_to_image_{true};

};
}
#endif // QSHADEREVALATOR_H
