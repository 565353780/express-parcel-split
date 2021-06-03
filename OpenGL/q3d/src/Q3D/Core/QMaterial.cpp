#include "QMaterial.h"
#include <qgl.h>
namespace GCL {

QMaterial::QMaterial(QObject *parent) : QOpenGLShaderProgram(parent)
{
    this->initializeOpenGLFunctions();
}

QMaterial::~QMaterial()
{
    this->clearTextures();
}

void QMaterial::clearTextures()
{
    foreach (auto texture, local_textures_) {
        if(texture->isCreated()){
            texture->destroy();
        }
    }
    local_textures_.clear();

    for(auto tid : local_texture_ids_)
    {
       if(glIsTexture(tid))
       {
           glDeleteTextures(1,&tid);
       }
    }
    local_texture_ids_.clear();
}

void QMaterial::linkShaders(const QString &vert_filename, const QString &frag_filename)
{
    this->addShaderFromSourceFile(QOpenGLShader::Vertex, vert_filename);
    this->addShaderFromSourceFile(QOpenGLShader::Fragment,frag_filename);

    this->link();
}

void QMaterial::addUniformValue(const char *name, GLfloat f)
{
    UniformType ut;
    ut.f_val_ = f;
    ut.type_ = UniformType::Float;
    uniform_map_[name] = ut;
}

void QMaterial::addUniformValue(const char *name, GLint val)
{
    UniformType ut;
    ut.i_val_ = val;
    ut.type_ = UniformType::Int;
    uniform_map_[name] = ut;
}

void QMaterial::addUniformTexture(const char *name, GLuint tex_id, bool add_to_local)
{
    UniformType ut;
    ut.tex_id_ = tex_id;
    ut.type_ = UniformType::Texture;
    uniform_map_[name] = ut;
    if(add_to_local)
    {
        local_texture_ids_.push_back(tex_id);
    }

}

GLuint QMaterial::addUniformTextureImage(const char *name, const QImage &image, QOpenGLTexture::Filter minfilter, QOpenGLTexture::Filter magfilter, QOpenGLTexture::WrapMode warpmode)
{    
    std::shared_ptr< QOpenGLTexture > texture(new QOpenGLTexture(image));
    texture->create();
    texture->bind();
    texture->setMinificationFilter(minfilter);
    texture->setMagnificationFilter(magfilter);
    texture->setWrapMode(warpmode);

    addUniformTexture(name,texture->textureId());

    local_textures_.push_back(texture);

    return texture->textureId();
}

QVector4D QMaterial::packInt(uint val)
{
    QVector4D v;
    v.setX(val % 256);
    val /= 256;
    v.setY(val % 256);
    val /= 256;
    v.setZ(val % 256);
    val /= 256;
    v.setW(255 - val % 256);

    return v / 255.0;
}

uint QMaterial::unpackInt(QVector4D v)
{
    uint sum = 0;
    v *= 255.0;
    sum += int(v.x());
    sum += int(v.y()) * 256;
    sum += int(v.z()) * 256 * 256;
    sum += (255 - int(v.w())) * 256 * 256 * 256;
    return sum;
}

QVector4D QMaterial::pack(double val)
{
    if(val >= 1.0) val = 0.999999;
    if(val < 0) val = 0;
    QVector4D bit_shift(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    QVector4D bit_mask(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    QVector4D res = (val * bit_shift);
    for(int i=0; i < 4; i++)
    {
        res[i] = res[i] - floor(res[i]);
    }
    res -= QVector4D(res.x(),res.x(),res.y(),res.z()) * bit_mask;
    return QVector4D(res.w(),res.z(),res.y(),res.x());
}

double QMaterial::unpack(QVector4D v)
{
    QVector4D bit_shift(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    double depth = QVector4D::dotProduct(QVector4D(v.w(),v.z(),v.y(),v.x()) , bit_shift);
    return depth;
}

void QMaterial::setUniforms(int &active_texture_count)
{
    for(auto iter = uniform_map_.cbegin(); iter != uniform_map_.cend(); iter++)
    {
        const UniformType& ut = iter.value();
        if(ut.type_ == UniformType::Float)
        {
            this->setUniformValue(iter.key().toLocal8Bit().data(),ut.f_val_);
        }
        else if(ut.type_ == UniformType::Int)
        {
            this->setUniformValue(iter.key().toLocal8Bit().data(),ut.i_val_);
        }
        else if(ut.type_ == UniformType::Texture)
        {
            glActiveTexture(GL_TEXTURE0+active_texture_count);
            glBindTexture(GL_TEXTURE_2D,ut.tex_id_);
            this->setUniformValue(iter.key().toLocal8Bit().data(),active_texture_count);
            active_texture_count++;
        }
    }
}

}
