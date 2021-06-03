#include "QPointMapModelFinder.h"
#include "Evaluator/QShaderEvaluator.h"
#include "Core/Q3DScene.h"
#include "Meshes/QPointMapMesh.h"
#include "Core/QMesh3D.h"
#include <ctime>
#include <QOpenGLFramebufferObject>
#include "Core/QMaterial.h"
#include <algorithm>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include "Meshes/QAxesMesh.h"
#include "Core/QMatrix2Eular.h"
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

    if(tfbo_buffer_)
    {
        delete tfbo_buffer_;
        tfbo_buffer_ = nullptr;
    }

    this->destroy();
}

void QPointMapModelFinder::create()
{
    if(m_context_) return;
    if(m_surface_) return;

    QSurfaceFormat surfaceFmt;
    if(QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        surfaceFmt.setRenderableType(QSurfaceFormat::OpenGL);
    }
    else
    {
        surfaceFmt.setRenderableType(QSurfaceFormat::OpenGLES);
    }


    m_context_ = new QOpenGLContext();

    m_context_->setFormat(surfaceFmt);


    surfaceFmt.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(surfaceFmt);


    m_context_->create();
    m_surface_ = new QOffscreenSurface();
    m_surface_->create();

    this->makeCurrent();
}

void QPointMapModelFinder::destroy()
{
    if(m_surface_)
    {
        m_surface_->deleteLater();
        m_surface_ = nullptr;
    }
    if(m_context_)
    {
        m_context_->deleteLater();
        m_context_ = nullptr;

    }
}

void QPointMapModelFinder::makeCurrent()
{
    if(m_context_)
    {
        m_context_->makeCurrent(m_surface_);
    }
}

void QPointMapModelFinder::doneCurrent()
{
    if(m_context_)
    {
        m_context_->doneCurrent();
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

    axes_ = new QAxesMesh(scene_);
    axes_->init();
    axes_->setScale(QVector3D(0.5,0.5,0.5));
    axes_->setRadius(5);
    axes_->setLength(50);
    if(!tfbo_buffer_)
    {
        int tfbo_size = 2048;
        tfbo_buffer_ = new QOpenGLFramebufferObject(tfbo_size,tfbo_size,QOpenGLFramebufferObject::CombinedDepthStencil);
    }

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
    this->makeCurrent();
    pointmap_->loadPLY(filename,w,h,zflip,flip_z);
    need_update_buffer_ = true;
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
//    z_map_.save("z_map.png");
    pointmap_->renderMap(normal_buffer_,QPointMapMesh::Normal_Map, projection_matrix,default_matrix);
//    normal_buffer_->toImage().save("normal.png");
    mesh_->getMaterial()->addUniformValue("pack_range", float(pack_range_));
    mesh_->getMaterial()->addUniformTexture("z_map",z_buffer_->texture());
    mesh_->getMaterial()->addUniformTexture("nor_map",normal_buffer_->texture());
    mesh_->getMaterial()->addUniformValue("u_state",int(1));
    mesh_->getMaterial()->addUniformValue("u_mscore",float(u_mscore_));
    mesh_->getMaterial()->addUniformValue("u_trunc",float(u_trunc_));

    qDebug()<<(clock() - t0)/(CLOCKS_PER_SEC/1000)<<"ms: update buffer";


}

void QPointMapModelFinder::findModels(const QList<QVector3D> &sample_points)
{

    int group_size = std::min(u_group_size_,sample_points.size());
    seeds_.clear();
    for(int i=0; i < group_size; i++)
    {
        visited_points_.push_back(sample_points[i]);
        SeedPoint seed = getSeedBySamplePoint(sample_points[i]);
        for(int k=0; k < 8; k++)
        {
            seed.eular_ = QVector3D(0,0,45*k);
            seeds_.push_back(seed);
        }
    }

    int iter = 3;
    int o_result_size = results_.size();
    float score_level = u_score_threshold_;
    u_scissor_level_ = score_level * 0.;
    while(iter--)
    {
        if(iter == 0)
        {
            u_scissor_level_ = score_level;
        }
        findAtSeeds();
        seeds_.clear();

        if(iter==0) break;

        for(int i=o_result_size; i < results_.size(); i++)
        {
            ResultTransform rt = results_[i];
            SeedPoint seed;
            seed.score_ = 0.0;
            seed.s_pos_ = rt.s_pos_;
            seed.offset_ = rt.offset_;
            seed.o_pos_ = seed.offset_;

            for(int k=0; k < 8; k++)
            {
                seed.eular_ = rt.eular_ + QVector3D(0,0,45*k);
                seeds_.push_back(seed);
            }
        }
        while(results_.size() > o_result_size) results_.pop_back();
    }
    u_score_threshold_ = score_level;

}

void QPointMapModelFinder::findModels()
{
    if(need_update_buffer_)
    {
        this->updateBuffer();
        need_update_buffer_ = false;
    }
    clock_t t0 = clock();

    QList<QVector3D> sample_points;
    getSamplePoints(sample_points);
    for(int i=0; i < sample_points.size(); i++)
    {
        SeedPoint seed = getSeedBySamplePoint( sample_points[i]);
        sample_points[i].setZ(seed.o_pos_.z());
    }

    for(int i=0; i < sample_points.size();)
    {
        bool flag = false;
        for(const QVector3D& p : visited_points_)
        {
            if((p-sample_points[i]).length() < 0.00001)
            {
                flag = true;
                break;
            }
        }

        if(flag)
        {
            std::swap(sample_points[i],sample_points[sample_points.length() - 1]);
            sample_points.pop_back();
        }
        else
        {
            i++;
        }
    }

    std::sort(sample_points.begin(),sample_points.end(),[&](const QVector3D &v0, const QVector3D &v1){
        return (getScene()->project(v0).z()< getScene()->project(v1).z());
    });





    findModels(sample_points);


//    tfbo_buffer_->toImage(false).save("tt.png");

    qDebug()<<(clock() - t0)/(CLOCKS_PER_SEC/1000)<<"ms: find models";
    qDebug()<<sample_points.size()<<" "<<results_.size()<<" ";
}

void QPointMapModelFinder:: quickFindAllModels()
{
    clock_t t0 = clock();
    if(need_update_buffer_)
    {
        this->updateBuffer();
        need_update_buffer_ = false;
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
                findAtSeeds();
            }
            seeds_.clear();
        }
        QVector3D sp = sample_points[i];
        SeedPoint seed = getSeedBySamplePoint(sp);
        for(int k=0; k < 8; k++)
        {
            seed.eular_ = QVector3D(0,0,45 * k);
            seeds_.push_back(seed);
        }
    }
    if(seeds_.size() > 0)
    {
        findAtSeeds();
    }
    seeds_.clear();
    qDebug()<<(clock() - t0)/(CLOCKS_PER_SEC/1000)<<"ms: find models";
    qDebug()<<sample_points.size()<<" "<<results_.size()<<" ";
    glFlush();
}

void QPointMapModelFinder::findAtSeeds()
{
    int tfbo_size = 2048;
    int ssize = 32;
    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();
    clock_t t0 = clock();
    mesh_->getMaterial()->addUniformValue("u_state",int(1));
    qreal aspect = this->width() / (qreal)(this->height());
    int tw = tfbo_buffer_ ->width() / ssize;
    int th = tfbo_buffer_->height() / ssize;


    QList<QVector4D> dvs;

//    dvs.push_back(QVector4D(0,0,0,0));
    dvs.push_back(QVector4D(1,0,0,0));
    dvs.push_back(QVector4D(-1,0,0,0));
    dvs.push_back(QVector4D(0,1,0,0));
    dvs.push_back(QVector4D(0,-1,0,0));

    dvs.push_back(QVector4D(0,0.5,0,1));
    dvs.push_back(QVector4D(0,-0.5,0,1));
    dvs.push_back(QVector4D(4,0,0,1));
    dvs.push_back(QVector4D(-4,0,0,0));

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

    int iter = u_local_iteration_num_;

    int bk_level = 50;
    QVector4D bkcolor = QMaterial::packInt(bk_level);


    int scissor_level = 4;
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
        else if(iter == 1)
        {
            dvs.clear();
            for(int k=0; k < 16; k++)
            {
                dvs.push_back(QVector4D(0,k / 2 * 5.0 * ((k % 2)?1:-1),0,1));
            }
        }
        else if(iter == 2)
        {
            dvs.clear();
            for(int k=0; k < 16; k++)
            {
                dvs.push_back(QVector4D(k / 2 * 5.0 * ((k % 2)?1:-1),0,0,1));
            }
        }
        tfbo_buffer_->bind();
        glViewport(0,0,tfbo_buffer_->width(),tfbo_buffer_->height());
        glClearColor(bkcolor[0], bkcolor[1],bkcolor[2],bkcolor[3]);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_TEXTURE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
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
            int dtx = tw * scissor_level * seed.s_pos_.x();
            int dty = tw/aspect * scissor_level * seed.s_pos_.y();
            glViewport(cx-dtx,cy-dty,tw*scissor_level,tw/aspect*scissor_level);

            glEnable(GL_SCISSOR_TEST);
            glScissor(cx-tw/2,cy-tw/2,tw,tw);

            QVector4D v = dvs[sj];

            if(v.w() > 0.5)
            {
                mesh_->setEuler(seed.eular_ + QVector3D(v.x(),v.y(),v.z()) * angle_scale );
                mesh_->setOffset(seed.offset_);
            }
            else
            {
                mesh_->setEuler(seed.eular_);
                mesh_->setOffset(seed.offset_ + QVector3D(v.x(),v.y(),v.z()) * offset_scale);
            }
            t_offset_list.push_back(mesh_->getOffset());
            t_eular_list.push_back(mesh_->getEuler());
            mesh_->render(projection_matrix,default_matrix);
            glDisable(GL_SCISSOR_TEST);
        }
        glClearColor(0,0,0,1);

        tfbo_buffer_->bindDefault();
        glViewport(0,0,this->width(),this->height());

        evaluator_->setToImage(false);
        evaluator_->getSumTexture(tfbo_buffer_->texture(),tfbo_buffer_->width(),tfbo_buffer_->height(), QShaderEvaluator::Size_4096);
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

                val = (val - (tw*tw*bk_level)) / (tw * tw / aspect * scissor_level * scissor_level);
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
        QVector2D s_pos;
        for(int k=0; k < 8; k++)
        {
            int kk = 8 * i + k;
            if(seeds_[kk].score_ > mval)
            {
                mval = seeds_[kk].score_;
                offset = seeds_[kk].offset_;
                eular = seeds_[kk].eular_;
                s_pos = seeds_[kk].s_pos_;
            }
        }
        qreal score =mval;
        if(score < u_score_threshold_)
        {
            continue;
        }
        ResultTransform rt;
        rt.score_ = score;
        rt.offset_ = offset;
        rt.eular_ = eular;        
        rt.s_pos_ = s_pos;

        bool flag = false;
        for(int ri=0; ri < results_.size(); ri++)
        {
            ResultTransform rt0 = results_[ri];

            QVector3D dt = rt0.offset_-rt.offset_;
            dt[2] = 0;
            if((dt).length() < 30)
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

        rt.eular_ = refineEulerIfSymmetry(rt.eular_);

        results_.push_back(rt);

        mesh_->setOffset(offset);
        mesh_->setEuler(eular);
    }
    glClearColor(0,0,0,1);
    if(results_.size()>0)
    {
        mesh_->setOffset(results_.back().offset_);
        mesh_->setEuler(results_.back().eular_);
    }
}

qreal QPointMapModelFinder::localSearching(QVector2D sp)
{

    seeds_.clear();
    SeedPoint seed;
    seed.s_pos_ = sp;
    seed.o_pos_ = mesh_->getOffset();
    seed.offset_ = seed.o_pos_;

    for(int k=0; k < 8; k++)
    {
        if(k < 8)
        {
            seed.eular_ = mesh_->getEuler()+ QVector3D(0,0,45*k);
        }
//        else
//        {
//            seed.eular_ = mesh_->getEular()+ QVector3D(45*(k%8),0,0);
//        }
        seeds_.push_back(seed);
    }

    int o_iter_num =  u_local_iteration_num_;
    float o_score_level = u_score_threshold_;
    u_local_iteration_num_ = 8;
    u_score_threshold_ = 0.0;

    int result_size = results_.size();
    findAtSeeds();

    float score = 0.0;
    while(results_.size() > result_size)
    {
         ResultTransform rt = results_.back();

         if(rt.score_ > score)
         {
             mesh_->setEuler(rt.eular_);
             mesh_->setOffset(rt.offset_);
             score = rt.score_;
         }

         results_.pop_back();
    }

    u_local_iteration_num_ = o_iter_num;
    u_score_threshold_ = o_score_level;

//    tfbo_buffer_->toImage(true).save("tt.png");
    return score;
}

void QPointMapModelFinder::setSamplePoints(QList<QVector3D> &sample_points)
{
    u_sample_points_ = sample_points;
}


void QPointMapModelFinder::getSamplePoints(QList<QVector3D> &sample_points)
{
    if(u_sample_points_.size()>0)
    {
        sample_points = u_sample_points_;
        return;
    }

    QOpenGLFramebufferObject  b_fbo(z_buffer_->width(),z_buffer_->height(), QOpenGLFramebufferObject::CombinedDepthStencil);
    b_fbo.bind();
    glViewport(0,0,b_fbo.width(),b_fbo.height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
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
    for(int i=1; i < image.width() - 1; i++)
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
        if(queue.size() > u_part_least_size_ *this->width() / b_fbo.width())
        {
            tblist.append(queue);
        }
        else
        {
            for(auto qp : queue)
            {
                image.setPixel(qp.x(),qp.y(),qRgb(0,0,0));
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
    for(int i=1; i < image.width() - 1; i++)
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
        if(queue.size() < u_part_least_size_ *this->width() / b_fbo.width())
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
        image.setPixel(center.x()+1,center.y(),qRgb(255,125,125));
        image.setPixel(center.x()-1,center.y(),qRgb(255,125,125));
        image.setPixel(center.x(),center.y()+1,qRgb(255,125,125));
        image.setPixel(center.x(),center.y()-1,qRgb(255,125,125));
        image.setPixel(center.x()+1,center.y()+1,qRgb(255,125,125));
        image.setPixel(center.x()-1,center.y()+1,qRgb(255,125,125));
        image.setPixel(center.x()+1,center.y()-1,qRgb(255,125,125));
        image.setPixel(center.x()-1,center.y()-1,qRgb(255,125,125));
        sample_points.push_back(QVector3D(center.x() * this->width() / b_fbo.width(),center.y() *this->height() / b_fbo.height(),0));
    }
    if(is_debug_mode_)
    {
       QImage image1 =  b_fbo.toImage(true);
       for(int i=0; i < image.width(); i++)
       {
           for(int j=0; j < image.height(); j++)
           {
               QRgb cc= image.pixel(i,j);
               if(cc==qRgb(255,255,0)||cc==qRgb(255,255,1)||cc==qRgb(255,125,125)) continue;
               QRgb c1 =image1.pixel(i,j);

               cc = qRgb(  (qRed(cc) + qRed(c1))>>1,  (qGreen(cc) + qGreen(c1))>>1, (qBlue(cc) + qBlue(c1))>>1);
               image.setPixel(i,j,cc);
           }
       }

       image.save("b.png");
    }
    u_sample_points_ = sample_points;
}

void QPointMapModelFinder::setDepthRange(float max_depth, float min_depth)
{
    u_max_depth_ = max_depth;
    u_min_depth_ = min_depth;
}

void QPointMapModelFinder::setScoreThreshold(float score)
{
    u_score_threshold_ = score;
}

void QPointMapModelFinder::setBoundaryExtendLevel(int level)
{
    u_boundary_level_ = level;
}

void QPointMapModelFinder::render()
{    
    if(need_update_buffer_)
    {
        this->updateBuffer();
        need_update_buffer_ = false;
    }
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

    QVector3D eular =  mesh_->getEuler();
    QVector3D offset = mesh_->getOffset();
    for(auto rt : results_)
    {
        mesh_->setOffset(rt.offset_ - u_local_pickpos_.normalized());
        mesh_->setEuler(rt.eular_);
        mesh_->render(projection_matrix,model_matrix);

        axes_->setEuler(rt.eular_);
        axes_->setOffset(rt.offset_ + axes_->localRotate(u_local_pickpos_));
        axes_->render(projection_matrix,model_matrix);
    }
    eular = refineEulerIfSymmetry(eular);
    qDebug()<<eular;
    mesh_->setEuler(eular);
    mesh_->setOffset(offset);
    axes_->setEuler(eular);
    axes_->setOffset(offset+axes_->localRotate(u_local_pickpos_));
    axes_->render(projection_matrix,model_matrix);
}

void QPointMapModelFinder::clearResults()
{
    visited_points_.clear();
    results_.clear();
    u_sample_points_.clear();
}


QList<QPointMapModelFinder::ResultTransform> QPointMapModelFinder::getResults() const
{
    return results_;
}

QVector3D QPointMapModelFinder::refineEulerIfSymmetry(QVector3D euler)
{
    if(NotSymmetry == symetry_type_)
    {
        return euler;
    }
    QVector3D dir = this->getScene()->getDefaultRotationMatrix().map(QVector3D(0,0,1));

    QMatrix4x4 m;
    m.rotate(euler[0],QVector3D(1,0,0));
    m.rotate(euler[1],QVector3D(0,1,0));
    m.rotate(euler[2],QVector3D(0,0,1));

    QVector3D v = m.map(u_local_pickpos_);

    if(v.z() * dir.z() > 0)
    {
        return euler;
    }

    if(X_Axis_180 & symetry_type_)
    {
        QMatrix4x4 mm;
        mm.rotate(180, QVector3D(1,0,0));
        mm = m * mm;
        QVector3D vv = mm.map(u_local_pickpos_);
        if(vv.z() * dir.z() > 0)
        {
            v  = vv;
            euler = QMatrix2Euler::fromMatrixToEuler(mm,"ZYX");
            return euler;
        }
    }

    if(Y_Axis_180 & symetry_type_)
    {
        QMatrix4x4 mm = m;
        mm.rotate(180, QVector3D(0,1,0));
        QVector3D vv = mm.map(u_local_pickpos_);
        if(vv.z() > v.z())
        {
            v = vv;
            euler = QMatrix2Euler::fromMatrixToEuler(mm,"ZYX");
        }
    }

    if(Z_Axis_180 & symetry_type_)
    {

        QMatrix4x4 mm = m;
        mm.rotate(180, QVector3D(0,0,1));
        QVector3D vv = mm.map(u_local_pickpos_);
        if(vv.z() > v.z())
        {
            v = vv;
            euler = QMatrix2Euler::fromMatrixToEuler(mm,"ZYX");
        }
    }

    return euler;

}

QPointMapModelFinder::SeedPoint QPointMapModelFinder::getSeedBySamplePoint(const QVector3D &sp)
{
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

    QVector3D poffset = this->getScene()->getDefaultRotationMatrix().map(u_local_pickpos_);

    seed.o_pos_ = intersect - poffset;
    seed.offset_ = seed.o_pos_;

    return seed;
}

QMatrix4x4 QPointMapModelFinder::ResultTransform::getTransform() const
{
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.translate(offset_);
    matrix.rotate(eular_[0],QVector3D(1,0,0));
    matrix.rotate(eular_[1],QVector3D(0,1,0));
    matrix.rotate(eular_[2],QVector3D(0,0,1));
    return matrix;
}

QQuaternion QPointMapModelFinder::ResultTransform::getQuat() const
{
    QMatrix4x4 matrix = getTransform();
    QQuaternion quat;
    quat = quat.fromRotationMatrix(matrix.normalMatrix());
    return quat;

}

}
