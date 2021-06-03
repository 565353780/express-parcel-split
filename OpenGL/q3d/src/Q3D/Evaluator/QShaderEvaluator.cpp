#include "QShaderEvaluator.h"
#include "Core/QMaterial.h"
#include "Meshes/QQuadMesh.h"
#include <QOpenGLFramebufferObject>
#include <ctime>
#include <iostream>
namespace GCL {

QShaderEvaluator::QShaderEvaluator(QObject *parent):QObject(parent), material_(new QMaterial(parent))
{
   this->initializeOpenGLFunctions();
   this->initMaterial();

    for(int i=0; i < 13; i++)
    {
        int size = 1 << i;
        QOpenGLFramebufferObject *fbo = new QOpenGLFramebufferObject(size,size);
        fbos_[size] = fbo;
    }
}

QShaderEvaluator::~QShaderEvaluator()
{
    for(auto fbo : fbos_)
    {
        if(fbo)
            delete fbo;
    }
    fbos_.clear();

}




double QShaderEvaluator::countTexture(GLuint texture_id, int w, int h, double vmin, double vmax, QShaderEvaluator::MapSize map_size)
{
    Problem problem;
    problem.w_ = w;
    problem.h_ = h;
    problem.texture_id_ = texture_id;
    problem.val_type_ = VT_Pack;
    problem.func_type_ = Func_Count;
    problem.map_size_ = map_size;
    problem.valid_min_ = vmin;
    problem.valid_max_ = vmax;

    return evalTexture(problem);
}

double QShaderEvaluator::getSumTexture(GLuint texture_id, int w, int h, QShaderEvaluator::MapSize map_size)
{
    Problem problem;
    problem.w_ = w;
    problem.h_ = h;
    problem.texture_id_ = texture_id;
    problem.val_type_ = VT_Pack;
    problem.func_type_ = Func_Sum;
    int esize = map_size;
    while(esize > 2 && esize > problem.w_)
    {
        esize /= 2;
    }
    problem.map_size_ = esize;

    return evalTexture(problem);
}

double QShaderEvaluator::evalTexture(const QShaderEvaluator::Problem &problem)
{
    if(!material_) return 0.0;


    QQuadMesh quad(this->material_);
    clock_t t0 = clock();

//    QOpenGLFramebufferObject fbo(problem.map_size_,problem.map_size_);
    glViewport(0,0,problem.map_size_,problem.map_size_);
    glBindTexture(GL_TEXTURE_2D,problem.texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//    qDebug()<<clock() - t0<<"ms";


    // render origin texture
    this->material_->addUniformTexture("texture", problem.texture_id_);
    this->material_->addUniformValue("u_state",int(0));
    this->material_->addUniformValue("texture_width",float(problem.w_));
    this->material_->addUniformValue("texture_height",float(problem.h_));
    this->material_->addUniformValue("u_width",float(problem.map_size_));
    this->material_->addUniformValue("u_height",float(problem.map_size_));
    this->material_->addUniformValue("func_type",int(problem.func_type_));
    this->material_->addUniformValue("val_type",int(problem.val_type_));
    this->material_->addUniformValue("valid_min",float(problem.valid_min_));
    this->material_->addUniformValue("valid_max",float(problem.valid_max_));
    this->material_->addUniformValue("pack_range",float(pack_range_));

    this->renderFBO(fbos_[problem.map_size_],this->material_,&quad);

    QImage rimg;
    GLuint tid = fbos_[problem.map_size_]->texture();
    this->material_->addUniformValue("u_state",int(2));
    this->material_->addUniformTexture("texture",tid);
    if(problem.func_type_ == Func_Count)
    {
        this->material_->addUniformValue("val_type",int(VT_Pack));
    }


//    fbo.release();

    int t_map_size = problem.map_size_;

    while(t_map_size > 1)
    {
        t_map_size /= 2;
//        QOpenGLFramebufferObject fbo1(t_map_size,t_map_size);
        this->material_->addUniformValue("u_width",float(t_map_size));
        this->material_->addUniformValue("u_height",float(t_map_size));
        this->material_->addUniformValue("texture_width",float(t_map_size*2));
        this->material_->addUniformValue("texture_height",float(t_map_size*2));
        glViewport(0,0,t_map_size,t_map_size);

        this->renderFBO(fbos_[t_map_size] ,this->material_,&quad);


//        fbo1.toImage().mirrored().save("res/res"+QString::number(t_map_size)+".png");
        if(t_map_size ==1)
        {
//            qDebug()<<clock() - t0<<"ms";

            if(need_to_image_)
            {
                rimg = fbos_[t_map_size]->toImage(); // 取值 需要20ms左右, 所以要想办法不去取值
                break;
            }
            else
            {
                break;
            }
        }
        GLuint tid =  fbos_[t_map_size]->texture();
//        texture_list.push_back(tid);
        glBindTexture(GL_TEXTURE_2D,tid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        this->material_->addUniformTexture("texture",tid);

    }
    if(!need_to_image_)
    {
        return 0.0;
    }
    double ans = 0.0;
    for(int i=0; i < rimg.width(); i++)
    {
        for(int j=0; j < rimg.height(); j++)
        {
            double val = 0.0;
            if(problem.val_type_ == VT_Pack || problem.func_type_ == Func_Count)
            {

                QRgb rgb = rimg.pixel(i,j);
                QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
//                qDebug()<<vv;

                vv /= 255.0;
                val = (QMaterial::unpackInt(vv));

                if(problem.func_type_ == Func_Count)
                {
                    val = int(val+0.5);
                }
            }
            else if(problem.val_type_ == VT_Red)
            {
                val = qRed(rimg.pixel(i,j)) / 255.0;

            }

            if(problem.func_type_ == Func_Max)
            {
                ans = val > ans?val:ans;
            }
            else if(problem.func_type_ == Func_Count || problem.func_type_ == Func_Sum)
            {
//                ans = val > ans?val:ans;
                    ans += val;
//                    qDebug()<<QMaterial::packInt(ans)*255.0;

            }
        }
    }
//    qDebug()<<"Size: "<<rimg.width() * rimg.height();
//    qDebug()<<clock() - t0<<"ms";
    return ans;
}

double QShaderEvaluator::evalTexture(GLuint texture_id, int val_type, int func_type, int w, int h, MapSize map_size)
{

    Problem problem;
    problem.w_ = w;
    problem.h_ = h;
    problem.texture_id_ = texture_id;
    problem.val_type_ = val_type;
    problem.func_type_ = func_type;
    problem.map_size_ = map_size;

    return evalTexture(problem);
}

void QShaderEvaluator::renderFBO(QOpenGLFramebufferObject *fbo, QMaterial *material, QMesh3D *mesh)
{
    fbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
//    material->bind();
//    QMatrix4x4 matrix;
//    matrix.setToIdentity();
//    material->setUniformValue("mvp_matrix", matrix);
//    int active_texture_count = 0;
//    material->setUniforms(active_texture_count);
    mesh->render();
}

void QShaderEvaluator::initMaterial()
{
    bool has_error = false;
    if(!has_error && !material_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/evalator_vshader.glsl"))
    {
        has_error = true;
    }
    // Compile fragment shader
    if (!has_error && !material_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/evalator_fshader.glsl"))
    {
        has_error = true;
    }
    if (!has_error && !material_->link())
    {
        has_error = true;
    }
    if(has_error)
    {
        material_->deleteLater();
        material_ = nullptr;
    }
}

qreal QShaderEvaluator::getPackRange() const
{
    return pack_range_;
}

void QShaderEvaluator::setPackRange(const qreal &r)
{
    pack_range_ = r;
}

GLuint QShaderEvaluator::getResultTextureId()
{
    return fbos_[1]->texture();
}

QOpenGLFramebufferObject *QShaderEvaluator::getResultFrameBufferObj()
{
    return fbos_[1];
}

QOpenGLFramebufferObject *QShaderEvaluator::getFrameBufferObj(QShaderEvaluator::MapSize s)
{
    return fbos_[int(s)];
}

void QShaderEvaluator::setToImage(bool t)
{
    need_to_image_ = t;
}
}
