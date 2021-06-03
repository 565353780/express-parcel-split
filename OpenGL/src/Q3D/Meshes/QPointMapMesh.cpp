#include "Meshes/QPointMapMesh.h"
#include <QImage>
#include "Core/QMaterial.h"
#include <QOpenGLFramebufferObject>
#include <ctime>
#include "Meshes/QQuadMesh.h"
#include "Evaluator/QShaderEvaluator.h"
//#include"tiffio.h"
#include<vector>
#include <iostream>
#include <Core/Q3DScene.h>
namespace GCL {

QPointMapMesh::QPointMapMesh(QObject *parent):QMesh3D(parent)
{
    default_vshader_filename_ = ":/shaders/pointmap_vshader.glsl";
    default_fshader_filename_ = ":/shaders/pointmap_fshader.glsl";
    this->init();
}

QPointMapMesh::QPointMapMesh(QMaterial *material, QObject *parent):QMesh3D(material,parent)
{
    default_vshader_filename_ = ":/shaders/pointmap_vshader.glsl";
    default_fshader_filename_ = ":/shaders/pointmap_fshader.glsl";
    this->init();
}

void QPointMapMesh::init()
{
    QMesh3D::init();


}


void QPointMapMesh::setPointMap(int w, int h, const std::vector<float> &points, const std::vector<uchar> &colors)
{
    clock_t t0 = clock();
    QImage color_image(w,h,QImage::Format_RGB888);
//    for(int j=0; j < h; j++)
//    {
//        for(int i=0; i < w; i++)
//        {
//            int ii = j * w + i;
//            color_image.setPixel(i,j,qRgb(colors[3 *ii],colors[3 *ii+1],colors[3 *ii+2]));
//        }
//    }


    QImage x_image(w,h,QImage::Format_RGBA8888);
    QImage y_image(w,h,QImage::Format_RGBA8888);
    QImage z_image(w,h,QImage::Format_RGBA8888);
     float pack_range = 256 * 8;
    for(int j=0; j < h ; j++)
    {
        for(int i=0; i < w; i++)
        {
            int ii = j * w + i;
            float z = points[3 *ii + 2];
            QVector4D v = QMaterial::pack((z / pack_range + 1.0) * 0.5);

            v *= 255;
            z_image.setPixel(i,j,qRgba(v[0],v[1],v[2],v[3]));
            float x = points[3 *ii];
            v =  QMaterial::pack((x / pack_range + 1.0) * 0.5);
            v *= 255;
            x_image.setPixel(i,j,qRgba(v[0],v[1],v[2],v[3]));

            float y = points[3 *ii+1];
            v =  QMaterial::pack((y / pack_range + 1.0) * 0.5);
            v *= 255;
            y_image.setPixel(i,j,qRgba(v[0],v[1],v[2],v[3]));


        }
    }

//    qDebug()<<"Init XYZ Image:"<<clock() - t0<<"ms";

    this->material_->addUniformTextureImage("texture",color_image);
    this->material_->addUniformTextureImage("z_map",z_image);
    this->material_->addUniformTextureImage("y_map", y_image);
    this->material_->addUniformTextureImage("x_map", x_image);

    this->material_->addUniformValue("pack_range",float(pack_range));
    this->material_->addUniformValue("u_width",w);
    this->material_->addUniformValue("u_height",h);
    this->material_->addUniformValue("has_normal",int(0));
    this->material_->addUniformValue("use_point_map",int(0));
    this->material_->addUniformValue("u_state",int(0));

    QImage nor_img;


    this->updateNormalImage(x_image,y_image,z_image,nor_img);

//    qDebug()<<"update Normal Image:"<<clock() - t0<<"ms";

    std::vector<VertexData> datalist;
    std::vector<FaceData> indices;
    int ptime = 1;
    int ow =w;
    int oh = h;
    w /= ptime;
    h /= ptime;
    datalist.reserve(w * h);

    for(int i=0; i < w * h; i++)
    {

        int x = i % w;
        int y = i / w;
        int ii = (y * ptime) * ow + (x * ptime);
        datalist.push_back(VertexData(QVector3D(points[3 * ii],points[3 *ii+1],points[3*ii+2]),
                QVector3D(),QVector4D(colors[3 *ii] / 255.0,colors[3 * ii+ 1] / 255.0, colors[3 * ii+2]/255.0,1.0),
                QVector2D(x/qreal(w),y/qreal(h))));

    }

    for(int i=0; i < w *h ;i++)
    {
        int x = i % w;
        int y = i / w;
        int ii = (y * ptime) * ow + (x * ptime);
        if(x < w-1 && y < h-1
                && points[3 * ii + 2] > -9999 && points[3 * (ii+1*ptime) + 2] > -9999
                && points[3 * (ii+ow*ptime+1*ptime) + 2] > -9999 && points[3 *(ii+ow*ptime) +2] > -9999)

//        if(x < w-1 && y < h-1)
        {
            indices.push_back(y * w + x);
            indices.push_back((y+1) * w + (x+1));
            indices.push_back(y * w + (x+1));

            indices.push_back((y+1) * w + (x+1));
            indices.push_back(y * w + x);
            indices.push_back((y+1) * w + x);
        }
    }

    this->material_->addUniformValue("u_width",w);
    this->material_->addUniformValue("u_height",h);
    this->material_->addUniformValue("has_normal",int(1));
    this->material_->addUniformValue("use_point_map",int(0));
    this->updateArrayBuffer(datalist);
    this->updateIndexBuffer(indices.data(),indices.size(),GL_TRIANGLES);
    qDebug()<<clock() - t0<<"ms: setting pointmap";
}

void QPointMapMesh::render(const QMatrix4x4 &project_matrix, const QMatrix4x4 &model_matrix)
{
    QMesh3D::render(project_matrix,model_matrix);


    //    this->material_->addUniformValue("u_state",int(3));

    //    int viewport[4];
    //    glGetIntegerv(GL_VIEWPORT,viewport);
    //    QOpenGLFramebufferObject fbo(pointmap_w_,pointmap_h_);
    //    glViewport(0,0,pointmap_w_,pointmap_h_);
    //    fbo.bind();
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    glEnable(GL_TEXTURE);
    //    glEnable(GL_DEPTH_TEST);

    //    QMesh3D::render(matrix);

    //    QShaderEvalator evalator;
    ////    qDebug()<<(evalator.getSumTexture(fbo.texture(),pointmap_w_,pointmap_h_)) / (pointmap_h_ * pointmap_w_);
    //    fbo.bindDefault();
    //    glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);

}

void QPointMapMesh::updateNormalImage(const QImage &x_map, const QImage &y_map, const QImage &z_map, QImage &nor_map)
{
    clock_t t0 = clock();

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    glViewport(0,0,x_map.width(),y_map.height());

    QOpenGLFramebufferObject fbo(x_map.size());
    quad_mesh_ = new QQuadMesh(this->material_,this);
    quad_mesh_->setBackground(true);
    GLuint tid = 0;
    // Filling hole

    this->material_->addUniformValue("u_state",int(1));
    renderFBO(&fbo,this->material_,quad_mesh_);
    this->material_->addUniformTexture("invalid_map",fbo.takeTexture(),true);
    this->material_->addUniformValue("u_state",int(2));

    fbo.release();
    for(int i=0; i < 10; i++)
    {
        renderFBO(&fbo,this->material_,quad_mesh_);

    //    nor_map = fbo.toImage(true);

        material_->addUniformValue("has_normal",int(1));

        if(tid > 0)
        {
            glBindTexture(GL_TEXTURE_2D,tid);
            glDeleteTextures(1,&tid);
        }
        tid = fbo.takeTexture();
        material_->addUniformTexture("nor_map",tid);
    }
    material_->addUniformTexture("nor_map",tid,true);



    fbo.bindDefault();
    glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    this->material_->addUniformValue("u_state",int(3));


    pointmap_w_ = x_map.width();
    pointmap_h_ = y_map.height();

    this->material_->addUniformValue("u_state",int(4));
//    qDebug()<<"Update Normal Image:"<<clock() - t0<<"ms";


}

void QPointMapMesh::loadPLY(const QString &filename, int w, int h, bool zflip,float flip_z)
{
    FILE *fp = fopen(filename.toLocal8Bit(),"rb");
    if(!fp){
        std::cout<<"Error: can't open "<<filename.toStdString()<<std::endl;
        return;
    }
    char buffer[255];
    bool is_binary = false;
    int n_vertex = 0;
    while(fgets(buffer,255,fp)!=nullptr)
    {
        if(strncmp(buffer,"format",6)==0)
        {
            strtok(buffer," ");
            char *p = strtok(nullptr," ");
            if(strncmp(p,"binary_little_endian",14)==0)
            {
                is_binary = true;
            }
        }
        else if(strncmp(buffer,"element vertex",14)==0)
        {
            strtok(buffer," ");
            char *p = strtok(nullptr," ");
            p = strtok(nullptr," ");
            n_vertex = atoi(p);
        }
        else if(strncmp(buffer,"end_header",10)==0)
        {
            break;
        }
    }
    using Vec3 = QVector3D;
    using Vec3i = QVector3D;
    std::vector<Vec3> vlist;
    std::vector<Vec3> plist;
    std::vector<Vec3i> clist;
    Vec3 vmin(999999,999999,999999);
    Vec3 vmax = -vmin;
    vlist.clear();
    clist.clear();
    for(int i=0; i < n_vertex; i++)
    {
        float v[3];
        unsigned char c[3];
        fread(v,sizeof(float),3,fp);
        fread(c,sizeof(unsigned char),3,fp);
        Vec3 p(v[0],v[1],v[2]);
        Vec3i color((int)c[0],(int)c[1],(int)c[2]);
        if(zflip)
        {
            p[2] =flip_z-p[2];
        }
        else
        {
            p[2] = p[2] + flip_z;
        }
        Vec3 p1 = p;
        if(p.length() > 0.1)
        {
//            qDebug()<<p;

            for(int k=0; k < 3; k++)
            {
                vmin[k] = std::min(vmin[k],p[k]);
                vmax[k] = std::max(vmax[k],p[k]);
            }
            vlist.push_back(p1);
            plist.push_back(p);
            clist.push_back(color);
        }


    }

    int ww = w;
    int hh = h;
    std::vector<float> points(3 * ww * hh, -99999);
    std::vector<uchar> colors(3 * ww * hh);
    qDebug()<<"PointMapRange: "<<vmin<<" "<<vmax<<" "<<(vmin+vmax)*0.5;
    for(int i=0; i < vlist.size(); i++)
    {
        Vec3 p = plist[i];
        int x = (p[0] - vmin[0]) / (vmax[0] - vmin[0]) * (ww-1);
        int y = (p[1] - vmin[1]) / (vmax[1] - vmin[1]) * (hh-1);
        for(int k=0; k < 3; k++)
        {
            points[3 * (y * ww + x) + k] = p[k];
        }
    }

    this->setPointMap(ww,hh,points,colors);

}

void QPointMapMesh::renderMap(QOpenGLFramebufferObject *fbo, int mapType, const QMatrix4x4 &project_matrix, const QMatrix4x4 &model_matrix)
{
    if(!fbo) return;
    if(mapType == Z_Map)
    {
        this->material_->addUniformValue("u_state",int(5));
    }
    else if(mapType == X_FloatMap)
    {
        this->material_->addUniformValue("u_state",int(7));
        this->material_->addUniformValue("u_map_type",int(0));

    }
    else if(mapType == Y_FloatMap)
    {
        this->material_->addUniformValue("u_state",int(7));
        this->material_->addUniformValue("u_map_type",int(1));

    }
    else if(mapType == Z_FloatMap)
    {
        this->material_->addUniformValue("u_state",int(7));
        this->material_->addUniformValue("u_map_type",int(2));

    }
    else if(mapType == Normal_Map)
    {
        this->material_->addUniformValue("u_state",int(6));
    }
    fbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    QMesh3D::render(project_matrix,model_matrix);
    fbo->release();

}

void QPointMapMesh::renderFBO(QOpenGLFramebufferObject *fbo, QMaterial *material, QMesh3D *mesh, const QMatrix4x4 &matrix)
{
    fbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    material->bind();
    material->setUniformValue("mvp_matrix", matrix);
    int active_texture_count = 0;
    material->setUniforms(active_texture_count);
    mesh->render();
}

}
