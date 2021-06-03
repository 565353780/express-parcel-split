#include "QPointMapModelFinder.h"
#include "Evaluator/QShaderEvaluator.h"
#include "Core/Q3DScene.h"
#include "Meshes/QPointMapMesh.h"
#include "Core/QMesh3D.h"
#include <ctime>
#include <QOpenGLFramebufferObject>
#include "Core/QMaterial.h"
namespace GCL {

QPointMapModelFinder::QPointMapModelFinder(Q3DScene *scene, QObject *parent):QObject(parent),scene_(scene)
{
    if(!scene_)
    {
        scene_ = new Q3DScene(this);
    }





}

QPointMapModelFinder::~QPointMapModelFinder()
{
    if(z_buffer_)
    {
        delete z_buffer_;
        z_buffer_ = nullptr;
    }

    if(normal_buffer_)
    {
        delete normal_buffer_;
        normal_buffer_ = nullptr;
    }
}

void QPointMapModelFinder::init()
{
    this->initializeOpenGLFunctions();
    scene_->init();
    pointmap_ = new QPointMapMesh(scene_);
    pointmap_->setObjectName("pointmap");

    mesh_ = new QMesh3D(scene_);
    mesh_->setObjectName("findmodel");
    mesh_->initShader(":/shaders/findmodel_vshader.glsl",":/shaders/findmodel_fshader.glsl");
    evaluator_ = new QShaderEvaluator(this);
}

void QPointMapModelFinder::resize(int w, int h)
{
    scene_->resize(w,h);
    if(z_buffer_)
    {
        delete z_buffer_;
        z_buffer_ = nullptr;
    }

    if(normal_buffer_)
    {
        delete normal_buffer_;
        normal_buffer_ = nullptr;
    }
    z_buffer_ = new QOpenGLFramebufferObject(w,h,QOpenGLFramebufferObject::CombinedDepthStencil);
    normal_buffer_ = new QOpenGLFramebufferObject(w,h,QOpenGLFramebufferObject::CombinedDepthStencil);
    width_ = w;
    height_ = h;
    need_update_buffer_ = true;
}

void QPointMapModelFinder::setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors)
{
    pointmap_->setPointMap(w,h,points,colors);
    need_update_buffer_ = true;


}

void QPointMapModelFinder::loadPointMapPLY(const QString &filename, int w, int h, bool zflip, float flip_z)
{
    pointmap_->loadPLY(filename,w,h,zflip,flip_z);
}

void QPointMapModelFinder::loadModel(const QString &filename)
{
    mesh_->loadFile(filename,true);
}

Q3DScene *QPointMapModelFinder::getScene()
{
    return scene_;
}

QMesh3D *QPointMapModelFinder::getMesh()
{
    return mesh_;
}

void QPointMapModelFinder::updateBuffer()
{
    if(!pointmap_ || !mesh_)
    {
        return;
    }
    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 model_matrix = getScene()->getModelMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();

    clock_t t0 = clock();

    glViewport(0,0,z_buffer_->width(),z_buffer_->height());

    pointmap_->getMaterial()->addUniformValue("pack_range", float(pack_range_));
    pointmap_->renderMap(z_buffer_,QPointMapMesh::Z_Map, projection_matrix,default_matrix);
    pointmap_->getMaterial()->addUniformValue("u_max_depth",u_max_depth_);
    pointmap_->getMaterial()->addUniformValue("u_min_depth",u_min_depth_);

    z_map_ = z_buffer_->toImage(true);
    pointmap_->renderMap(normal_buffer_,QPointMapMesh::Normal_Map, projection_matrix,default_matrix);

    mesh_->getMaterial()->addUniformValue("pack_range", float(pack_range_));
    mesh_->getMaterial()->addUniformTexture("z_map",z_buffer_->texture());
    mesh_->getMaterial()->addUniformTexture("nor_map",normal_buffer_->texture());
    mesh_->getMaterial()->addUniformValue("u_state",int(1));
    mesh_->getMaterial()->addUniformValue("u_mscore",float(u_mscore_));
    mesh_->getMaterial()->addUniformValue("u_trunc",float(u_trunc_));

    qDebug()<<(clock() - t0)/(CLOCKS_PER_SEC/1000)<<"ms: update buffer";


}

void QPointMapModelFinder::findModels()
{
    clock_t t0 = clock();
    if(need_update_buffer_)
    {
        this->updateBuffer();
    }

    results_.clear();

    QList<QVector3D> sample_points;
    results_.clear();
    getSamplePoints(sample_points);
    seeds_.clear();
    for(int i=0; i < sample_points.size(); i++)
    {
        if(i % 8 == 0)
        {
            if(seeds_.size() > 0)
            {
                findBySeeds();
            }
            seeds_.clear();
        }
        QVector3D sp = sample_points[i];
        SeedPoint seed;
        QRgb rgb = z_map_.pixel(sp.x(),sp.y());
        QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
        vv /= 255;
        qreal zval = QMaterial::unpackInt(vv) / 100.0;
        QVector3D raypoint,raydir;
        getScene()->getScreenRay(sp.x(), z_map_.height()- sp.y(), raypoint,raydir);
        qreal t0 = QVector3D::dotProduct(raydir ,QVector3D(0,0,1));
        qreal t1 =  QVector3D::dotProduct((raypoint - QVector3D(0,0,zval)) , QVector3D(0,0,1));
        qreal k = -t1 / t0;
        QVector3D intersect = raypoint + k * raydir;

        seed.s_pos_ = QVector2D(sp.x() / z_map_.width(),1.0-sp.y() / z_map_.height());
        seed.o_pos_ = intersect+QVector3D(0,0,18);
        seed.offset_ = seed.o_pos_;
        for(int k=0; k < 8; k++)
        {
            seed.eular_ = QVector3D(90,45*k,0);
            seeds_.push_back(seed);
        }
    }
    if(seeds_.size() > 0)
    {
        findBySeeds();
    }
    seeds_.clear();
    qDebug()<<(clock() - t0)/(CLOCKS_PER_SEC/1000)<<"ms: find models";
    qDebug()<<sample_points.size()<<" "<<results_.size()<<" ";
}

void QPointMapModelFinder::findBySeeds()
{
    int tfbo_size = 2048;
    int ssize = 32;
    QOpenGLFramebufferObject tfbo(tfbo_size,tfbo_size,QOpenGLFramebufferObject::CombinedDepthStencil);
    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();
    clock_t t0 = clock();
    mesh_->getMaterial()->addUniformValue("u_state",int(1));
    qreal aspect = this->width() / (qreal)(this->height());
    int tw = tfbo.width() / ssize;
    int th = tfbo.height() / ssize;


    QList<QVector4D> dvs;

    dvs.push_back(QVector4D(1,0,0,0));
    dvs.push_back(QVector4D(-1,0,0,0));
    dvs.push_back(QVector4D(0,1,0,0));
    dvs.push_back(QVector4D(0,-1,0,0));

    dvs.push_back(QVector4D(1,1,0,0));
    dvs.push_back(QVector4D(-1,1,0,0));
    dvs.push_back(QVector4D(1,-1,0,0));
    dvs.push_back(QVector4D(-1,-1,0,0));

    dvs.push_back(QVector4D(0,0,2,0));
    dvs.push_back(QVector4D(0,0,-2,0));

    dvs.push_back(QVector4D(1,0,0,1));
    dvs.push_back(QVector4D(-1,0,0,1));
    dvs.push_back(QVector4D(0,1,0,1));
    dvs.push_back(QVector4D(0,-1,0,1));
    dvs.push_back(QVector4D(0,0,1,1));
    dvs.push_back(QVector4D(0,0,-1,1));

    double angle_scale = 12;
    double offset_scale = 2.0;

    int iter = 14;
    while(iter--)
    {
        if(iter == 0)
        {
            dvs.clear();
            for(int k=0; k < 16; k++)
            {
                dvs.push_back(QVector4D(0,0,k / 2 * 5.0 * ((k % 2)?1:-1),1));
            }
        }


        tfbo.bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QList<QVector3D> t_offset_list;
        QList<QVector3D> t_eular_list;
        for(int i=0; i < ssize * ssize; i++)
        {
            int cx = tw * (i % ssize) + tw * 0.5;
            int cy = th * (i / ssize) + th * 0.5;

            int si = i / 16;
            int sj = i % 16;
            if(si >= seeds_.size()) continue;
            SeedPoint seed = seeds_[si];
            int dtx = tw * 4 * seed.s_pos_.x();
            int dty = tw/aspect * 4 * seed.s_pos_.y();
            glViewport(cx-dtx,cy-dty,tw*4,tw/aspect*4);

            glEnable(GL_SCISSOR_TEST);
            glScissor(cx-tw/2,cy-tw/2,tw,tw);

            QVector4D v = dvs[sj];

            if(v.w() > 0.5)
            {
                mesh_->setEular(seed.eular_ + QVector3D(v.x(),v.y(),v.z()) * angle_scale );
                mesh_->setOffset(seed.offset_);
            }
            else
            {
                mesh_->setEular(seed.eular_);
                mesh_->setOffset(seed.offset_ + QVector3D(v.x(),v.y(),v.z()) * offset_scale);
            }
            t_offset_list.push_back(mesh_->getOffset());
            t_eular_list.push_back(mesh_->getEular());
            mesh_->render(projection_matrix,default_matrix);
            glDisable(GL_SCISSOR_TEST);
        }

        tfbo.bindDefault();
        glViewport(0,0,this->width(),this->height());

        evaluator_->setToImage(false);
        evaluator_->getSumTexture(tfbo.texture(),tfbo.width(),tfbo.height(), QShaderEvaluator::Size_4096);
        QImage res = evaluator_->getFrameBufferObj(QShaderEvaluator::MapSize(ssize))->toImage(false);
        double mval = 0.0;

         for(int j=0; j < res.height(); j++)
         {
            for(int i=0; i < res.width(); i++)
            {
                QRgb rgb = res.pixel(i,j);

                QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
                vv /= 255.0;
                double val = (QMaterial::unpackInt(vv));
                int index = j * res.width() + i;
                int si = index / 16;
                int sj = index % 16;
                if(val > mval){
                    mval =val;
                }
                if(si >= seeds_.size()) continue;
                if(seeds_[si].score_ < val)
                {

                    seeds_[si].score_ = val;
                    seeds_[si].offset_ = t_offset_list[index];
                    seeds_[si].eular_ = t_eular_list[index];
                }
            }
        }


    }

    for(int i=0; i < seeds_.size() / 8; i++)
    {
        qreal mval = 0.0;
        QVector3D offset ;
        QVector3D eular;
        for(int k=0; k < 8; k++)
        {
            int kk = 8 * i + k;
            if(seeds_[kk].score_ > mval)
            {
                mval = seeds_[kk].score_;
                offset = seeds_[kk].offset_;
                eular = seeds_[kk].eular_;
            }
        }
        qreal score = mval / (tw * tw / aspect * 16);
        qDebug()<<score;
        if(score < u_score_level_)
        {
            continue;
        }
        ResultTransform rt;
        rt.score_ = score;
        rt.offset_ = offset - QVector3D(0,0,1);
        rt.eular_ = eular;

        bool flag = false;
        for(int ri=0; ri < results_.size(); ri++)
        {
            ResultTransform rt0 = results_[ri];

            QVector3D dt = rt0.offset_-rt.offset_;
            dt[2] = 0;
            if((dt).length() < 10)
            {
                if(rt.score_ > rt0.score_)
                {
                    results_[ri] = rt;
                }
                flag = true;
                break;
            }

        }
        if(flag) continue;
        results_.push_back(rt);

        mesh_->setOffset(offset);
        mesh_->setEular(eular);
    }
    tfbo.toImage(false).save("tt.png");
    if(results_.size()>0)
    {
        mesh_->setOffset(results_.back().offset_);
        mesh_->setEular(results_.back().eular_);
    }
}


void QPointMapModelFinder::getSamplePoints(QList<QVector3D> &sample_points)
{
    QOpenGLFramebufferObject  b_fbo(this->width(),this->height());
    b_fbo.bind();
    glViewport(0,0,b_fbo.width(),b_fbo.height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    pointmap_->getMaterial()->addUniformValue("u_state",int(4));
    pointmap_->render(scene_->getProjectionMatrix(),scene_->getModelMatrix());
    b_fbo.bindDefault();
    QImage image = b_fbo.toImage(true);
    for(int i=0; i < image.width(); i++)
    {
        for(int j=0; j < image.height(); j++)
        {
            QRgb rgb = image.pixel(i,j);
            if(qRed(rgb) == 0 && qBlue(rgb)==0)
            {
                image.setPixel(i,j,qRgb(255,0,0));
            }
        }
    }
    QList<QVector2D> boundary_list;
    int dx[8] = {1,-1,0,0,1,1,-1,-1};
    int dy[8] = {0,0,1,-1,1,-1,1,-1};
    for(int i=image.width() * 0.1; i < image.width() - 1; i++)
    {
        for(int j=1; j < image.height() - 1; j++)
        {
            QRgb rgb = image.pixel(i,j);
            if(rgb == qRgb(255,255,0)) continue;
            if(rgb == qRgb(255,0,0)) continue;
            for(int k=0; k < 8; k++)
            {
                int px = i + dx[k];
                int py = j + dy[k];
                if(image.pixel(px,py) == qRgb(255,0,0))
                {

                   image.setPixel(i,j,qRgb(255,255,0));
                   boundary_list.push_back(QVector2D(i,j));
                   break;
                }
            }

        }
    }
    QList<QVector2D> tblist;
    for(auto p : boundary_list)
    {
        if(image.pixel(p.x(),p.y()) == qRgb(255,255,1))
        {
            continue;
        }
        QList<QVector2D> queue;
        queue.push_back(p);
        image.setPixel(p.x(),p.y(),qRgb(255,255,1));
        for(int qi=0; qi < queue.size(); qi++)
        {
            QVector2D p0 = queue[qi];
            for(int k=0; k < 8; k++)
            {
                int px = p0.x() + dx[k];
                int py = p0.y() + dy[k];
                if(px < 0 || px >= image.width()) continue;
                if(py < 0 || py >= image.height()) continue;
                if(image.pixel(px,py) == qRgb(255,255,0))
                {
                   image.setPixel(px,py,qRgb(255,255,1));
                   queue.push_back(QVector2D(px,py));
                }
            }
        }
        if(queue.size() > 20)
        {
            tblist.append(queue);
        }
        else
        {
            for(auto qp : queue)
            {
                image.setPixel(qp.x(),qp.y(),qRgb(255,0,0));
            }
        }
    }
    boundary_list = tblist;
    int target_level = u_boundary_level_;

    for(int level = 0; level < target_level; level++)
    {
        QList<QVector2D> tlist;
        for(auto p : boundary_list)
        {
            image.setPixel(p.x(),p.y(),qRgb(255,0,0));
        }

        for(QVector2D p : boundary_list)
        {
            for(int k=0; k < 8; k++)
            {
                int px = p.x() + dx[k];
                int py = p.y() + dy[k];
                if(px < 0 || px >= image.width()) continue;
                if(py < 0 || py >= image.height()) continue;
                if(image.pixel(px,py) != qRgb(255,0,0))
                {
                   image.setPixel(px,py,qRgb(255,0,0));
                   tlist.push_back(QVector2D(px,py));
                }
            }
        }
        boundary_list = tlist;
    }

    boundary_list.clear();
    for(int i=image.width() * 0.1; i < image.width() - 1; i++)
    {
        for(int j=1; j < image.height() - 1; j++)
        {
            QRgb rgb = image.pixel(i,j);
            if(rgb == qRgb(255,255,0)) continue;
            if(rgb == qRgb(255,0,0)) continue;
            for(int k=0; k < 8; k++)
            {
                int px = i + dx[k];
                int py = j + dy[k];
                if(image.pixel(px,py) == qRgb(255,0,0))
                {

                   image.setPixel(i,j,qRgb(255,255,0));
                   boundary_list.push_back(QVector2D(i,j));
                   break;
                }
            }

        }
    }
    for(QVector2D p : boundary_list)
    {
        image.setPixel(p.x(),p.y(),qRgb(255,255,0));
    }

    for(auto p : boundary_list)
    {
        if(image.pixel(p.x(),p.y()) == qRgb(255,255,1))
        {
            continue;
        }
        QList<QVector2D> queue;
        queue.push_back(p);
        image.setPixel(p.x(),p.y(),qRgb(255,255,1));
        for(int qi=0; qi < queue.size(); qi++)
        {
            QVector2D p0 = queue[qi];
            for(int k=0; k < 8; k++)
            {
                int px = p0.x() + dx[k];
                int py = p0.y() + dy[k];
                if(px < 0 || px >= image.width()) continue;
                if(py < 0 || py >= image.height()) continue;
                if(image.pixel(px,py) == qRgb(255,255,0))
                {
                   image.setPixel(px,py,qRgb(255,255,1));
                   queue.push_back(QVector2D(px,py));
                }
            }
        }
        if(queue.size() < 30 *this->width() / b_fbo.width())
        {
            continue;
        }
        QVector2D center;
        for(auto qp : queue)
        {
            center += qp;
        }
        center /= queue.size();
        image.setPixel(center.x(),center.y(),qRgb(255,125,125));
        sample_points.push_back(QVector3D(center.x() * this->width() / b_fbo.width(),center.y() *this->height() / b_fbo.height(),0));
    }
    glViewport(0,0,this->width(),this->height());

    image.save("b.png");
}

void QPointMapModelFinder::setDepthRange(float max_depth, float min_depth)
{
    u_max_depth_ = max_depth;
    u_min_depth_ = min_depth;
}

void QPointMapModelFinder::setScoreLevel(float score)
{
    u_score_level_ = score;
}

void QPointMapModelFinder::setBoundaryExtendLevel(int level)
{
    u_boundary_level_ = level;
}

void QPointMapModelFinder::render()
{
    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 model_matrix = getScene()->getModelMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();

    glViewport(0,0,this->width(),this->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    pointmap_->getMaterial()->addUniformValue("u_state",int(6));
    mesh_->getMaterial()->addUniformValue("u_state",int(2));

    pointmap_->render(projection_matrix,model_matrix);
    //        glDisable(GL_DEPTH_TEST);

    QVector3D eular =  mesh_->getEular();
    QVector3D offset = mesh_->getOffset();
    for(auto rt : results_)
    {
        mesh_->setOffset(rt.offset_);
        mesh_->setEular(rt.eular_);
        mesh_->render(projection_matrix,model_matrix);
    }
    mesh_->setEular(eular);
    mesh_->setOffset(offset);
}


QList<QPointMapModelFinder::ResultTransform> QPointMapModelFinder::results() const
{
    return results_;
}
}
