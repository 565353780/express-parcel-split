#include "QModelFinderWidget.h"
#include "Meshes/QPointMapMesh.h"
#include "Core/Q3DScene.h"
#include "Evaluator/QShaderEvaluator.h"
#include <QKeyEvent>
#include <QOpenGLFramebufferObject>
#include "Core/QMaterial.h"
#include <QDebug>
#include <QMouseEvent>
#include <iostream>
#include <ctime>
#include <QPainter>
namespace GCL {

QModelFinderWidget::QModelFinderWidget(QWidget *parent):QRenderWidget(parent)
{
    scene_->setRotationSpeed(2);
    scene_->setPanSpeed(40);
    scene_->setScaleSpeed(1);
    scene_->setProjectionParameters(0.1,10000.0,45);
    this->show();
}

QModelFinderWidget::~QModelFinderWidget()
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

void QModelFinderWidget::setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors)
{
//    pointmap_->loadTiff("1.tiff");
    pointmap_->setPointMap(w,h,points,colors);
}

void QModelFinderWidget::loadPointMapPLY(const QString &filename, int w , int h, bool zflip, float flip_z)
{
    pointmap_->loadPLY(filename,w,h,zflip,flip_z);

}

void QModelFinderWidget::loadModel(const QString &filename)
{
    mesh_->loadFile(filename,true);
}

void QModelFinderWidget::initializeGL()
{
    QRenderWidget::initializeGL();
    mesh_ = new QMesh3D(getScene());
    mesh_->initShader(":/shaders/findmodel_vshader.glsl",":/shaders/findmodel_fshader.glsl");

    pointmap_ = new QPointMapMesh(getScene());
    evaluator_ = new QShaderEvaluator(this);


    array_fbo_ = new QOpenGLFramebufferObject(24,1);
    //    z_buffer_ = new QOpenGLFramebufferObject(1024,1024);
    //    normal_buffer_ = new QOpenGLFramebufferObject(1024,1024);
    mesh_->setObjectName("findmodel");

}

void QModelFinderWidget::resizeGL(int w, int h)
{

    QRenderWidget::resizeGL(w,h);
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
    if(fbo_)
    {
        delete fbo_;
        fbo_ = nullptr;
    }
    z_buffer_ = new QOpenGLFramebufferObject(w,h,QOpenGLFramebufferObject::CombinedDepthStencil);
    normal_buffer_ = new QOpenGLFramebufferObject(w,h,QOpenGLFramebufferObject::CombinedDepthStencil);
    fbo_ = new QOpenGLFramebufferObject(w / 4,h / 4,QOpenGLFramebufferObject::CombinedDepthStencil);
    need_update_buffer_ = true;


}

void QModelFinderWidget::paintGL()
{
    if(!pointmap_ || !mesh_)
    {
        return;
    }

    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 model_matrix = getScene()->getModelMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();
    if(need_update_buffer_)
    {
        clock_t t0 = clock();

        glViewport(0,0,this->width(),this->height());

        pointmap_->getMaterial()->addUniformValue("pack_range", float(256 * 8));
        pointmap_->renderMap(z_buffer_,QPointMapMesh::Z_Map, projection_matrix,default_matrix);
        pointmap_->getMaterial()->addUniformValue("u_max_depth",float(u_max_depth_));
        pointmap_->getMaterial()->addUniformValue("u_min_depth",float(u_min_depth_));


        z_map_ = z_buffer_->toImage();
        pointmap_->renderMap(normal_buffer_,QPointMapMesh::Normal_Map, projection_matrix,default_matrix);
//        f_map0_ = normal_buffer_->toImage(true);
        mesh_->getMaterial()->addUniformValue("pack_range", float(256 * 8));
        mesh_->getMaterial()->addUniformTexture("z_map",z_buffer_->texture());
        mesh_->getMaterial()->addUniformTexture("nor_map",normal_buffer_->texture());
        mesh_->getMaterial()->addUniformValue("u_state",int(1));
        mesh_->getMaterial()->addUniformValue("u_mscore",float(400.0));
        mesh_->getMaterial()->addUniformValue("u_trunc",float(8.0));
        qDebug()<<clock() - t0<<"ms: update buffer";
        need_update_buffer_ = false;

    }
    makeCurrent();
    glViewport(0,0,this->width(),this->height());
    QOpenGLFramebufferObject::bindDefault();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    //    getScene()->render();
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

    mesh_->render(projection_matrix,model_matrix);

    glFlush();
}

void QModelFinderWidget::keyPressEvent(QKeyEvent *event)
{
    int offset = 20;
    if(event->key() == Qt::Key_W)
    {
        mesh_->rotateEular(QVector3D(offset,0,0));
    }
    else if(event->key() == Qt::Key_S)
    {
        mesh_->rotateEular(QVector3D(-offset,0,0));
    }
    else if(event->key() == Qt::Key_A)
    {
        mesh_->rotateEular(QVector3D(0,offset,0));
    }
    else if(event->key() == Qt::Key_D)
    {
        mesh_->rotateEular(QVector3D(0,-offset,0));
    }
    else if(event->key() == Qt::Key_Left)
    {
        mesh_->translate(QVector3D(offset,0,0));

    }
    else if(event->key() == Qt::Key_Right)
    {
        mesh_->translate(QVector3D(-offset,0,0));
    }
    else if(event->key() == Qt::Key_Up)
    {
        mesh_->translate(QVector3D(0,offset,0));
    }
    else if(event->key() == Qt::Key_Down)
    {
        mesh_->translate(QVector3D(0,-offset,0));
    }

    else if(event->key() == Qt::Key_9)
    {
        mesh_->translate(QVector3D(0,0,offset/2));
    }
    else if(event->key() == Qt::Key_0)
    {
        mesh_->translate(QVector3D(0,0,-offset/2));
    }
    else if(event->key() == Qt::Key_Space)
    {
        getScene()->setDefaultView();
    }
    else if(event->key() == Qt::Key_1)
    {
        QVector3D eular =  mesh_->getEular();
        mesh_->rotateEular(QVector3D(90,0,0)-eular);
    }
    else if(event->key() == Qt::Key_2)
    {
        QVector3D eular =  mesh_->getEular();
        mesh_->rotateEular(QVector3D(0,90,0)-eular);
    }
    else if(event->key() == Qt::Key_R)
    {
        qDebug()<<"score: "<<findModel();
    }
    else if(event->key() == Qt::Key_X)
    {
//        findAllModels();

        results_.clear();
    }
    else if(event->key() == Qt::Key_F)
    {
        findModel3();
    }
    else if(event->key() == Qt::Key_L)
    {
        qDebug()<<getScene()->getModelMatrix();
    }
    else if(event->key() == Qt::Key_T)
    {
//        qDebug()<<getScene()->getModelMatrix();
        testFindModel();
    }
    else if(event->key() == Qt::Key_P)
    {
        ResultTransform rt;
        double score = findModel();
        rt.eular_ = mesh_->getEular();
        rt.offset_ = mesh_->getOffset();
        rt.score_ = score;
        results_.push_back(rt);
    }
    else if(event->key() == Qt::Key_M)
    {
        findAllModel();
    }
    else if(event->key() == Qt::Key_N)
    {
        findAllModelBySeeds();
    }
    update();
}

void QModelFinderWidget::mousePressEvent(QMouseEvent *event)
{
    QRenderWidget::mousePressEvent(event);

    if(event->buttons() ==Qt::LeftButton)
    {

        QRgb rgb = z_map_.pixel(event->pos());               
        QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
        vv /= 255;
        qreal zval = QMaterial::unpackInt(vv) / 100.0;
        qDebug()<<zval;
        if(zval < 0.1)
        {
            return;
        }
        QVector3D raypoint,raydir;

        getScene()->getScreenRay(event->x(), this->height()- event->y(), raypoint,raydir);


        qreal t0 = QVector3D::dotProduct(raydir ,QVector3D(0,0,1));
        qreal t1 =  QVector3D::dotProduct((raypoint - QVector3D(0,0,zval)) , QVector3D(0,0,1));
        qreal k = -t1 / t0;
        QVector3D intersect = raypoint + k * raydir;
        qDebug()<<intersect<<event->pos();



        QVector3D eular =  mesh_->getEular();
        mesh_->rotateEular(QVector3D(90,0,0)-eular);
        QVector3D offset = mesh_->getOffset();
        mesh_->translate(intersect-offset + QVector3D(0,0,18));
        clock_t clock_0 = clock();


        double mval = findModel();
        double score =0.0;
        intersect = mesh_->getOffset();

        qDebug()<<score;
        qDebug()<<(clock() - clock_0)/(CLOCKS_PER_SEC / 1000)<<"ms";
        update();
    }
}

void QModelFinderWidget::findAllModel()
{

    clock_t t0 = clock();
    QList<QVector3D> sample_points;
    getSamplePoints(sample_points);
    results_.clear();
    for(QVector3D sp : sample_points)
    {
        QRgb rgb = z_map_.pixel(sp.x(),sp.y());
        QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
        vv /= 255;
        qreal zval = QMaterial::unpackInt(vv) / 100.0;
        QVector3D raypoint,raydir;

        getScene()->getScreenRay(sp.x(), this->height()- sp.y(), raypoint,raydir);


        qreal t0 = QVector3D::dotProduct(raydir ,QVector3D(0,0,1));
        qreal t1 =  QVector3D::dotProduct((raypoint - QVector3D(0,0,zval)) , QVector3D(0,0,1));
        qreal k = -t1 / t0;
        QVector3D intersect = raypoint + k * raydir;
//        qDebug()<<intersect<<" "<<sp;
        mesh_->setEular(QVector3D(90,0,0));



        mesh_->setOffset(intersect);

        double score =  findModel3();

        if(score > score_level_ * 0.5)
        {

            if(score < score_level_)
            {
                score =  findModel3();
                score = findModel();
                score = findModel();
                score = findModel();
                score = findModel();

            }
            else
            {
                score = findModel();
            }
            qDebug()<<"score:"<<score;

            if(score >score_level_ * 0.7)
            {
                ResultTransform rt;
                rt.eular_ = mesh_->getEular();
                rt.offset_ = mesh_->getOffset();
                rt.score_ = score;
                results_.push_back(rt);
            }

        }
    }


    qDebug()<<sample_points.size()<<" "<<results_.size()<<" "<< clock() - t0<<"ms";

//    image.save("b.png");


}

void QModelFinderWidget::findAllModelBySeeds()
{
    clock_t t0 = clock();

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
    qDebug()<<sample_points.size()<<" "<<results_.size()<<" "<< clock() - t0<<"ms";

}

void QModelFinderWidget::findBySeeds()
{
    this->makeCurrent();
    int tfbo_size = 2048;
    int ssize = 32;
    QOpenGLFramebufferObject tfbo(tfbo_size,tfbo_size,QOpenGLFramebufferObject::CombinedDepthStencil);
    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();
    clock_t t0 = clock();
    mesh_->getMaterial()->addUniformValue("u_state",int(1));
    qreal aspect = fbo_->width() / (qreal)(fbo_->height());
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

    int iter = 13;
//    QVector4D bkcolor = QMaterial::packInt(15);
//    glClearColor(bkcolor[0],bkcolor[1],bkcolor[2],bkcolor[3]);

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
                int index = j * res.width() + i;
                int si = index / 16;
                int sj = index % 16;
                if(si >= seeds_.size()) continue;

                double val = (QMaterial::unpackInt(vv));
                if(val < 0) val = 0;

                if(val > mval){
                    mval =val;
                }
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
        if(score <score_level_)
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
//    tfbo.toImage(false).save("tt.png");
    if(results_.size()>0)
    {
        mesh_->setOffset(results_.back().offset_);
        mesh_->setEular(results_.back().eular_);
    }
    glClearColor(0,0,0,1);

}

void QModelFinderWidget::getSamplePoints(QList<QVector3D> &sample_points)
{
    makeCurrent();
    QOpenGLFramebufferObject  b_fbo(this->width(),this->height(), QOpenGLFramebufferObject::CombinedDepthStencil);
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
            if(qRed(rgb) == 0 && qBlue(rgb)==0  && qBlue(rgb)==0)
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
        QVector2D vmin(image.width(),image.height());
        QVector2D vmax(0,0);
        for(int qi=0; qi < queue.size(); qi++)
        {
            QVector2D p0 = queue[qi];
            for(int k=0; k < 2; k++)
            {
                vmin[k] = std::min(p0[k],vmin[k]);
                vmax[k] = std::max(p0[k],vmax[k]);
            }
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
        QVector2D vt = vmax-vmin;
        if(vt.x() < vt.y())
        {
            std::swap(vt[0],vt[1]);
        }

        if(vt.x() > 15 && queue.size() > 5 * this->width() / fbo_->width())
        {
            tblist.append(queue);
        }
        else
        {
            for(auto qp : queue)
            {
                image.setPixel(qp.x(),qp.y(),qRgb(0,0,0));
                for(int k=0; k < 8; k++)
                {
                    int px = qp.x() + dx[k];
                    int py = qp.y() + dy[k];
                    if(px < 0 || px >= image.width()) continue;
                    if(py < 0 || py >= image.height()) continue;
                    if(image.pixel(px,py) == qRgb(255,0,0))
                    {
                       image.setPixel(px,py,qRgb(0,0,0));
                    }
                }
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
        if(queue.size() < u_part_least_size_ *this->width() / b_fbo.width())
        {
            for(auto qp : queue)
            {
                image.setPixel(qp.x(),qp.y(),qRgb(0,0,0));
            }
            continue;
        }
        QVector2D center;
        for(auto qp : queue)
        {
            center += qp;
        }
        center /= queue.size();
        image.setPixel(center.x(),center.y(),qRgb(255,255,255));
        sample_points.push_back(QVector3D(center.x() * this->width() / b_fbo.width(),center.y() *this->height() / b_fbo.height(),0));
    }
    glViewport(0,0,this->width(),this->height());
    QImage image1 = b_fbo.toImage(true);

    for(int i=0; i < image1.width(); i++)
    {
        for(int j=0; j < image1.height(); j++)
        {
            QRgb c0 = image.pixel(i,j);
            QRgb c1 = image1.pixel(i,j);

            image.setPixel(i,j,qRgb((qRed(c0)+qRed(c1))>>1,(qGreen(c0)+qGreen(c1))>>1,(qBlue(c0)+qBlue(c1))>>1));
        }
    }
        image.save("b.png");

}

double QModelFinderWidget::testFindModel()
{
    this->makeCurrent();
    int tfbo_size = 2048;
    int ssize = 32;
    QOpenGLFramebufferObject tfbo(tfbo_size,tfbo_size,QOpenGLFramebufferObject::CombinedDepthStencil);

    clock_t t0 = clock();

    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 model_matrix = getScene()->getModelMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();

//    default_matrix.translate();
    mesh_->getMaterial()->addUniformValue("u_state",int(1));

    QList<QVector4D> dvs;

    dvs.push_back(QVector4D(0.5,0,0,0));
    dvs.push_back(QVector4D(-0.5,0,0,0));
    dvs.push_back(QVector4D(0,0.5,0,0));
    dvs.push_back(QVector4D(0,-0.5,0,0));
    dvs.push_back(QVector4D(0,0,1,0));
    dvs.push_back(QVector4D(0,0,-1,0));

    dvs.push_back(QVector4D(1,0,0,1));
    dvs.push_back(QVector4D(-1,0,0,1));
    dvs.push_back(QVector4D(0,1,0,1));
    dvs.push_back(QVector4D(0,-1,0,1));
    dvs.push_back(QVector4D(0,0,1,1));
    dvs.push_back(QVector4D(0,0,-1,1));
    int iter = 1;

    double angle_scale = 12;
    double offset_scale = 4.0;
    tfbo.bind();
    qreal aspect = fbo_->width() / (qreal)(fbo_->height());
    int tw = tfbo.width() / ssize;
    int th = tfbo.height() / ssize;
//    getScene()->setModelMatrix(matrix);
    double posX = (416.0)/this->width();
    double posY = qreal(this->height() - 520.0) / this->height();

    int dtx = tw * 4 * posX;
    int dty = tw/aspect * 4 * posY;

    while(iter --)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double l_angle_scale = angle_scale;
        double l_offset_scale = offset_scale;



        for(int i=0; i < ssize * ssize; i++)
        {
            QVector4D v = dvs[i % dvs.size()];

//            glViewport(tw * (i % ssize),th * (i / ssize), tw,tw/aspect);
            int cx = tw * (i % ssize) + tw * 0.5;
            int cy = th * (i / ssize) + th * 0.5;
            glViewport(cx-dtx,cy-dty,tw*4,tw/aspect*4);

            glEnable(GL_SCISSOR_TEST);

            glScissor(cx-tw/2,cy-tw/2,tw,tw);
//            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            mesh_->rotateEular(QVector3D(0,15,0));
            mesh_->render(projection_matrix,default_matrix);

            glDisable(GL_SCISSOR_TEST);
        }
    }
    tfbo.bindDefault();
    glViewport(0,0,this->width(),this->height());

    evaluator_->setToImage(false);
    evaluator_->getSumTexture(tfbo.texture(),tfbo.width(),tfbo.height(), QShaderEvaluator::Size_4096);


//    for(int i=0; i < image.width(); i++)
//    {
//        for(int j=0; j < image.height(); j++){
//            QRgb rgb = image.pixel(i,j);

//            QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
//            vv /= 255.0;
//            double val = (QMaterial::unpackInt(vv));

//        }
//    }

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
            if(mval < val) mval = val;
//            qDebug()<<val;
        }
        break;
    }
    qDebug()<<mval<<" "<<mval/(tw * tw / aspect * 16.0);
    qDebug()<<clock() - t0<<"ms";
    QImage image = tfbo.toImage(false);

    image.save("testF.png");
    return 0.0;
}



double QModelFinderWidget::findModel()
{
    this->makeCurrent();
    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 model_matrix = getScene()->getModelMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();




    QList<QVector4D> dvs;

    dvs.push_back(QVector4D(0.5,0,0,0));
    dvs.push_back(QVector4D(-0.5,0,0,0));
    dvs.push_back(QVector4D(0,0.5,0,0));
    dvs.push_back(QVector4D(0,-0.5,0,0));
    dvs.push_back(QVector4D(0,0,1,0));
    dvs.push_back(QVector4D(0,0,-1,0));

    dvs.push_back(QVector4D(1,0,0,1));
    dvs.push_back(QVector4D(-1,0,0,1));
    dvs.push_back(QVector4D(0,1,0,1));
    dvs.push_back(QVector4D(0,-1,0,1));
    dvs.push_back(QVector4D(0,0,1,1));
    dvs.push_back(QVector4D(0,0,-1,1));

    int iter = 3;
    double mval = 0.0;

    double angle_scale = 12;
    double offset_scale = 4.0;
    fbo_->bind();
    glViewport(0,0,fbo_->width(), fbo_->height());
//    QVector4D bkcolor = QMaterial::packInt(15);
//    glClearColor(bkcolor[0],bkcolor[1],bkcolor[2],bkcolor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mesh_->render(projection_matrix,default_matrix);
    fbo_->bindDefault();
    while(iter --)
    {

        double l_angle_scale = angle_scale;
        double l_offset_scale = offset_scale;

        for(int i=0; i < dvs.size(); i++)
        {
            QVector4D v = dvs[i % dvs.size()];
            if(v.w() > 0.5)
            {
                mesh_->rotateEular(QVector3D(v.x(),v.y(),v.z())*l_angle_scale);
            }
            else
            {
                mesh_->translate(QVector3D(v.x(),v.y(),v.z()) * l_offset_scale);
            }
            if(i < dvs.size())
            {
                mesh_->getMaterial()->addUniformValue("u_state",int(1));
            }
            else
            {
                mesh_->getMaterial()->addUniformValue("u_state",int(3));
            }
            fbo_->bind();
            glViewport(0,0,fbo_->width(), fbo_->height());

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mesh_->render(projection_matrix,default_matrix);
            fbo_->bindDefault();

            if(v.w() > 0.5)
            {
                mesh_->rotateEular(-QVector3D(v.x(),v.y(),v.z())*l_angle_scale);
            }
            else
            {
                mesh_->translate(-QVector3D(v.x(),v.y(),v.z()) * l_offset_scale);
            }

            evaluator_->setToImage(false);
            evaluator_->getSumTexture(fbo_->texture(),fbo_->width(),fbo_->height(), QShaderEvaluator::Size_1024);

            QOpenGLFramebufferObject::blitFramebuffer(array_fbo_,QRect(i,0,1,1), evaluator_->getResultFrameBufferObj(),QRect(0,0,1,1));

        }
        QList<double> res;
        getArray(res);

        int mi = 0;
//        for(int i=0; i < dvs.size(); i++)
//        {
//            res[i] = res[i] *(res[i] / (res[i +dvs.size()] + 1e-7)) * 3.5;
//        }
        for(int i=0; i < dvs.size(); i++)
        {
            if(res[i] > res[mi])
            {
                mi = i;
            }
        }
        if(res[mi] < mval)
        {
            if(l_angle_scale > 0.01)
            {
                angle_scale *= 0.5;
            }
            if(l_offset_scale > 0.01)
            {
                offset_scale *= 0.5;
            }
            if(offset_scale < 1.5)
            {
                break;
            }
            continue;
        }


        mval = res[mi];

        if(mval < 10)
        {
            return mval;
        }

        QVector4D v = dvs[mi];
        if(v.w() > 0.5)
        {
            mesh_->rotateEular(QVector3D(v.x(),v.y(),v.z())*l_angle_scale);
        }
        else
        {
            mesh_->translate(QVector3D(v.x(),v.y(),v.z()) *l_offset_scale);
        }

    }
    //        printArray();

    glClearColor(0,0,0,1);

//    qDebug()<<mval;
    return mval / qreal(fbo_->width() *fbo_->height());

}

double QModelFinderWidget::findModel2()
{
    this->makeCurrent();
    QMatrix4x4 projection_matrix = getScene()->getProjectionMatrix();
    QMatrix4x4 model_matrix = getScene()->getModelMatrix();
    QMatrix4x4 default_matrix = getScene()->getDefaultModelMatrix();



    mesh_->getMaterial()->addUniformValue("u_state",int(1));
    double angle_scale = 10;
    double offset_scale = 4.0;

    int iter = 10;
    double mval = 0.0;


        // 先遍历所有角度
    QVector3D offset = mesh_->getOffset();

    QVector3D result_offset;
    QVector3D result_angle;
    int psize = 6;
    for(int i=0; i < psize; i++)
    {
        mesh_->setOffset(offset);
        mesh_->setEular(QVector3D(90,i * 360 / psize,0));

        double val = findModel();
        if(val > mval)
        {
            mval = val;
            result_angle = mesh_->getEular();
            result_offset = mesh_->getOffset();
        }


//        fbo_->bind();
//        glViewport(0,0,fbo_->width(), fbo_->height());
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        mesh_->render(projection_matrix,default_matrix);
//        fbo_->bindDefault();

//        evaluator_->setToImage(false);
//        evaluator_->getSumTexture(fbo_->texture(),fbo_->width(),fbo_->height(), QShaderEvaluator::Size_512);

//        QOpenGLFramebufferObject::blitFramebuffer(array_fbo_,QRect(i,0,1,1), evaluator_->getResultFrameBufferObj(),QRect(0,0,1,1));
    }


    mesh_->setEular(result_angle);
    mesh_->setOffset(result_offset);

    return mval;
//    QList<double> res;
//    getArray(res);

//    int mi = 0;
//    for(int i=0; i < res.size(); i++)
//    {
//        if(res[i] > res[mi])
//        {
//            mi = i;
//        }
//    }
//    mesh_->setEular(QVector3D(90,mi * 360 /  array_fbo_->width(),0));





//    return findModel();

//    evaluator_->setToImage(true);

    //    return evaluator_->getSumTexture(fbo_->texture(),fbo_->width(),fbo_->height(), QShaderEvaluator::Size_512);
}

double QModelFinderWidget::findModel3()
{
    int iter = 2;
    QVector3D eular = mesh_->getEular();
    QVector3D offset = mesh_->getOffset();

    QVector3D origin_offset = offset;
    qreal mval = 0.0;
    while(iter--)
    {
        for(int i=0; i<4; i++)
        {
            double val = findModel();
            if(mval < val)
            {
                mval = val;
                offset = mesh_->getOffset();
                eular = mesh_->getEular();
            }
            mesh_->rotateEular(QVector3D(0,90,0));
        }
        if(mval > score_level_) break;              
        mesh_->setOffset(origin_offset);
        mesh_->rotateEular(QVector3D(0,45,0));

    }
    mesh_->setOffset(offset);
    mesh_->setEular(eular);
    return mval;
}

void QModelFinderWidget::printArray()
{
    QImage image = array_fbo_->toImage();
    for(int j=0; j < image.height(); j++)
    {
        for(int i=0; i < image.width(); i++)
        {
            QRgb rgb = image.pixel(i,j);

            QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
            vv /= 255.0;
            double val = (QMaterial::unpackInt(vv));

            std::cout<<val<<" ";
        }
        std::cout<<std::endl;

    }
    std::cout<<std::endl;


}

void QModelFinderWidget::getArray(QList<double> &res)
{
    res.clear();
    QImage image = array_fbo_->toImage();
    for(int j=0; j < image.height(); j++)
    {
        for(int i=0; i < image.width(); i++)
        {
            QRgb rgb = image.pixel(i,j);

            QVector4D vv(qRed(rgb),qGreen(rgb),qBlue(rgb),qAlpha(rgb));
            vv /= 255.0;
            double val = (QMaterial::unpackInt(vv));
            res.push_back(val);
        }
    }
}

void QModelFinderWidget::setScoreLevel(const qreal &score_level)
{
    score_level_ = score_level;

}

void QModelFinderWidget::setDepthRange(float max_depth, float min_depth)
{
    u_max_depth_ = max_depth;
    u_min_depth_ = min_depth;
}

void QModelFinderWidget::setBoundaryExtendLevel(int level)
{
    u_boundary_level_ = level;
}

void QModelFinderWidget::setPartLeastSize(int size)
{
    u_part_least_size_ = size;
}


}
