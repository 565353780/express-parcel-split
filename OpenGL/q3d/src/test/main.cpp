#include <QApplication>
#include <QLabel>
#include <QSurfaceFormat>

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include "Core/Q3DScene.h"
#include "Meshes/QCubeMesh.h"
#include "Core/QMaterial.h"
#include <QWidget>
#include <QOpenGLFramebufferObject>
#include <QDebug>
#include "Widgets/QRenderWidget.h"
#include "Meshes/QQuadMesh.h"
#include "Meshes/QPointMapMesh.h"
#include "PointMapWidget.h"
#include "Evaluator/QShaderEvaluator.h"
#include <QImage>
#include <ctime>
#include "Widgets/QModelFinderWidget.h"
#include "Finder/QPointMapModelFinder.h"
#include <QDir>
#include <QFileInfo>

#include <gl/GLU.h>
#include <iostream>
#include <QtCore>
#include <fstream>
#include <QOpenGLBuffer>

#include "easymesh.h"

void loadPointClound(const QString &filename, int &w, int &h, std::vector<float> &points, std::vector<uchar> &colors)
{
    FILE *fp = fopen(filename.toLocal8Bit().data(), "rb");
    fread(&w,sizeof(w),1,fp);
    fread(&h,sizeof(h),1,fp);

    points.resize(3 * w * h);
    colors.resize(3 * w * h);
    QVector3D center;
    int count = 0;
    for(int j=0; j < h; j++)
    {
        for(int i=0; i < w; i++)
        {
            float v[3];
            int c[3];
            fread(v,sizeof(float),3,fp);
            fread(c,sizeof(int),3,fp);
            if(v[2] > 0)
            {
                center += QVector3D(v[0],v[1],v[2]);
                count++;
            }
            if(v[0]==0 && v[1] == 0 && v[2] == 0)
            {
                v[0] = -999999;
                v[1] = -999999;
                v[2] = -999999;
            }
            for(int k=0; k < 3; k++)
            {
                points[3 * (j * w + i) + k] = v[k];
                colors[3 * (j * w + i) + k] = c[k];
            }

        }
    }
    fclose(fp);
    center /= count;
//    qDebug()<<center;
}

void testEvaluator()
{
    GCL::QShaderEvaluator evalator;

//    evalator.setPackRange(256*256);
    QImage testimg(1024,1024,QImage::Format_ARGB32);

    double sum = 0.0;
    srand(clock());
    double sum1 = 0.0;
    for(int i=0; i < testimg.width(); i++)
    {
        for(int j=0; j < testimg.height(); j++)
        {
            double val = rand() % 2048;
            QVector4D vv = GCL::QMaterial::packInt(val);

            vv *= 255;
            testimg.setPixel(i,j,qRgba(vv.x(),vv.y(),vv.z(),vv.w()));

            sum += val;
            QRgb c0 = testimg.pixel(i,j);
            QVector4D v1(qRed(c0),qGreen(c0),qBlue(c0),qAlpha(c0));
            v1 /= 255.0;
            sum1 +=(GCL::QMaterial::unpackInt(v1));// GCL::QMaterial::unpackInt( QVector4D(floor(vv.x()),floor(vv.y()),floor(vv.z()),floor(vv.w())) / 255.0);

     //            if(val > sum) sum = val;
        }
    }

    qDebug()<<int(sum)<<" "<<int(sum1)<<" "<<(sum - sum1)<<" "<<GCL::QMaterial::packInt(sum)*255.0;


    QOpenGLTexture texture(testimg,QOpenGLTexture::DontGenerateMipMaps);
    texture.setMinificationFilter(QOpenGLTexture::Nearest);
    texture.setMagnificationFilter(QOpenGLTexture::Nearest);
    texture.create();
    GCL::QShaderEvaluator::Problem problem;
    problem.texture_id_ = texture.textureId();
    problem.w_ = testimg.width();
    problem.h_ = testimg.height();
    problem.val_type_ = GCL::QShaderEvaluator::VT_Pack;
    problem.func_type_ = GCL::QShaderEvaluator::Func_Sum;
    problem.valid_min_ = 60.0 / 255.0;
    problem.valid_max_ = 1.0;
    problem.map_size_ = GCL::QShaderEvaluator::Size_1024;
    double val1 = evalator.evalTexture(problem);

    double sumE = 0.0;
    for(int i=0; i < testimg.width(); i++)
    {
        for(int j=0; j <testimg.height(); j++)
        {
            QRgb c0 = testimg.pixel(i,j);
//            QRgb rgb = evalator.image_.pixel(i,j);

            QVector4D vv(qRed(c0),qGreen(c0),qBlue(c0),qAlpha(c0));

            vv /= 255.0;
            double val = (GCL::QMaterial::unpackInt(vv));
            sumE += val;
        }
    }
    qDebug()<<int(val1)<<" "<<int(sumE);
    qDebug()<<(val1 - sum)<<" "<<(val1 - sum) / (testimg.width() * testimg.height());
}

int testOnScreen()
{
    using namespace GCL;
    GCL::QModelFinderWidget rw;

//        testEvaluator();

//        return 0;
    rw.loadModel("c3.obj");

    GCL::Q3DScene *scene = rw.getScene();

    QMesh3D *mesh = scene->getMesh("findmodel");
    if(mesh)
    {
            mesh->translate(QVector3D(26.4,30,856.576));
            mesh->rotateEular(QVector3D(90,0,0));
    }
//    QMesh3D *mesh = new QMesh3D(scene);
//    mesh->loadFile("c1.obj",true);

//    mesh->init();
//    mesh->translate(QVector3D(26.4,30,856.576));
//    mesh->rotate(90,QVector3D(1,0,0));


    scene->lookat(QVector3D(0,0,1030.23),QVector3D(0,0,0),QVector3D(0,1,0));


    QMatrix4x4 matrix;
    matrix.lookAt(QVector3D(0,0,0),QVector3D(0,0,1),QVector3D(0,1,0));
//    matrix.translate(-26.4,-1,-156.576);
//    matrix.translate(-465.519,-88,-945.45);
//    matrix.translate(-473.4,-82,0.);

    matrix.translate(-613,-49,-216);
    scene->setDefaultModelMatrix(matrix);
    scene->setDefaultView();



//  传入点云图
    int w,h;
    std::vector<float> points;
    std::vector<uchar> colors;
    w = 1280 * 0.5;
    h = 1024 * 0.5;
//    loadPointClound("3.pimg",w,h,points,colors);

//    rw.setPointMap(w,h,points,colors);
//    rw.loadPointMapPLY("2.ply",w,h,true);
    rw.setBoundaryExtendLevel(14);
    rw.setPartLeastSize(100);
    rw.setDepthRange(758,600);
    rw.loadPointMapPLY("0000.ply",w,h,true,800);
    rw.setScoreLevel(2.2);
    rw.resize(w,h);

    qDebug()<<w<<" "<<h;

    return QApplication::exec();
}

int testOffScreen(QString filename)
{
    using namespace GCL;
    QOpenGLContext ctx;;
    QSurfaceFormat surfaceFmt;
    if(QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        surfaceFmt.setRenderableType(QSurfaceFormat::OpenGL);
    }
    else
    {
        surfaceFmt.setRenderableType(QSurfaceFormat::OpenGLES);
        //surfaceFmt.setVersion(3,0);
    }
    ctx.setFormat(surfaceFmt);
    bool b = ctx.create();
    surfaceFmt.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(surfaceFmt);

    QOffscreenSurface* pSurface = new QOffscreenSurface;
    pSurface->setFormat(surfaceFmt);
    pSurface->create();

    ctx.makeCurrent(pSurface);
    clock_t t0 = clock();
    int w,h;
    w = 1280 * 0.8;
    h = 1024 * 0.8;
    QPointMapModelFinder rw;

    rw.init();
    rw.resize(w,h);


    GCL::Q3DScene *scene = rw.getScene();

    // 设置相机矩阵
    QMatrix4x4 matrix;
    matrix.lookAt(QVector3D(0,0,0),QVector3D(0,0,1),QVector3D(0,1,0));
    matrix.translate(-613,-49,-216);

    scene->setDefaultModelMatrix(matrix);
    scene->setDefaultView();


    // 读入模型
    rw.loadModel("c3.obj");


    // 读入点云图
    rw.loadPointMapPLY(filename,w,h,true,800);



    // 设置得分阈值, 阈值以上保留
   rw.setScoreLevel(1.6);

    rw.setBoundaryExtendLevel(6);

    rw.setBoundaryExtendLevel(14);
    rw.setPartLeastSize(100);
    rw.setDepthRange(758,600);
    rw.setScoreLevel(2.2);
    //    finder.updateBuffer();
    rw.findModels();




    // 以下只是为了保存图片用
    QOpenGLFramebufferObject fbo(QSize(w,h),QOpenGLFramebufferObject::CombinedDepthStencil);

    fbo.bind();
    rw.render();

    fbo.bindDefault();
    qDebug()<<(clock() -t0)/(CLOCKS_PER_SEC/1000)<<"ms: Total";
    fbo.toImage().save("testOff.png");
//    filename.replace(QString("ply"),QString("png"));
//    fbo.toImage(false).save(filename);

//    return QApplication::exec();

    return 0;



}

void setGLFormat()
{
    QSurfaceFormat format;
    if(QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        format.setRenderableType(QSurfaceFormat::OpenGL);
    }
    else
    {
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        //surfaceFmt.setVersion(3,0);
    }
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
}

using namespace GCL;

std::vector<float> getRect3D(QMesh3D *mesh)
{
    //计算mesh对应世界坐标系的变换矩阵
    QMatrix4x4 transform_matrix;
    transform_matrix.setToIdentity();
    transform_matrix.translate(mesh->getOffset());

    QVector3D get_eular = mesh->getEular();
    transform_matrix.rotate(get_eular[0],QVector3D(1,0,0));
    transform_matrix.rotate(get_eular[1],QVector3D(0,1,0));
    transform_matrix.rotate(get_eular[2],QVector3D(0,0,1));

    //获取mesh的所有点集
    std::vector<QVector3D> mesh_points;

    QOpenGLBuffer *vertice_array = mesh->getVerticeArray();

    QMesh3D::VertexData *vertice_data = (QMesh3D::VertexData *)vertice_array->map(QOpenGLBuffer::ReadOnly);

    for(int i = 0; i < mesh->getVerticeSize(); ++i)
    {
        mesh_points.emplace_back(transform_matrix.map(vertice_data[i].position_));
    }

    vertice_array->unmap();

    float x_min = mesh_points[0].x();
    float x_max = x_min;
    float y_min = mesh_points[0].y();
    float y_max = y_min;
    float z_min = mesh_points[0].z();
    float z_max = z_min;

    for(int i = 1; i < mesh->getVerticeSize(); ++i)
    {
        if(mesh_points[i].x() < x_min)
        {
            x_min = mesh_points[i].x();
        }
        if(mesh_points[i].x() > x_max)
        {
            x_max = mesh_points[i].x();
        }
        if(mesh_points[i].y() < y_min)
        {
            y_min = mesh_points[i].y();
        }
        if(mesh_points[i].y() > y_max)
        {
            y_max = mesh_points[i].y();
        }
        if(mesh_points[i].z() < z_min)
        {
            z_min = mesh_points[i].z();
        }
        if(mesh_points[i].z() > z_max)
        {
            z_max = mesh_points[i].z();
        }
    }

    std::vector<float> rect3d;

    rect3d.emplace_back(x_min);
    rect3d.emplace_back(y_min);
    rect3d.emplace_back(z_min);
    rect3d.emplace_back(x_max);
    rect3d.emplace_back(y_max);
    rect3d.emplace_back(z_max);

    return rect3d;
}

std::vector<float> getEasyRect3D(EasyMesh *mesh)
{
    mesh->updateTransformMatrix();

    //获取mesh的所有点集
    std::vector<QVector3D> mesh_points;

    for(int i = 0; i < mesh->getVerticeSize(); ++i)
    {
        mesh_points.emplace_back(mesh->transform_matrix.map(mesh->vertex_list[i].position_));
    }

    float x_min = mesh_points[0].x();
    float x_max = x_min;
    float y_min = mesh_points[0].y();
    float y_max = y_min;
    float z_min = mesh_points[0].z();
    float z_max = z_min;

    for(int i = 1; i < mesh->getVerticeSize(); ++i)
    {
        if(mesh_points[i].x() < x_min)
        {
            x_min = mesh_points[i].x();
        }
        if(mesh_points[i].x() > x_max)
        {
            x_max = mesh_points[i].x();
        }
        if(mesh_points[i].y() < y_min)
        {
            y_min = mesh_points[i].y();
        }
        if(mesh_points[i].y() > y_max)
        {
            y_max = mesh_points[i].y();
        }
        if(mesh_points[i].z() < z_min)
        {
            z_min = mesh_points[i].z();
        }
        if(mesh_points[i].z() > z_max)
        {
            z_max = mesh_points[i].z();
        }
    }

    std::vector<float> rect3d;

    rect3d.emplace_back(x_min);
    rect3d.emplace_back(y_min);
    rect3d.emplace_back(z_min);
    rect3d.emplace_back(x_max);
    rect3d.emplace_back(y_max);
    rect3d.emplace_back(z_max);

    return rect3d;
}

bool normalizeMesh(QMesh3D *mesh)
{
    std::vector<float> rect3d = getRect3D(mesh);

    //mesh归一化到单位立方体中
    float center_x = (rect3d[0] + rect3d[3]) / 2;
    float center_y = (rect3d[1] + rect3d[4]) / 2;
    float center_z = (rect3d[2] + rect3d[5]) / 2;

    float max_length = rect3d[3] - rect3d[0];

    if(rect3d[4] - rect3d[1] > max_length)
    {
        max_length = rect3d[4] - rect3d[1];
    }
    if(rect3d[5] - rect3d[2] > max_length)
    {
        max_length = rect3d[5] - rect3d[2];
    }

    QOpenGLBuffer *vertice_array = mesh->getVerticeArray();

    QMesh3D::VertexData *vertice_data = (QMesh3D::VertexData *)vertice_array->map(QOpenGLBuffer::ReadWrite);

    for(int i = 0; i < mesh->getVerticeSize(); ++i)
    {
        vertice_data[i].position_.setX((vertice_data[i].position_.x() - center_x) / max_length);
        vertice_data[i].position_.setY((vertice_data[i].position_.y() - center_y) / max_length);
        vertice_data[i].position_.setZ((vertice_data[i].position_.z() - center_z) / max_length);
    }

    vertice_array->unmap();

    return true;
}

bool normalizeEasyMesh(EasyMesh *mesh)
{
    std::vector<float> rect3d = getRect3D(mesh);

    //mesh归一化到单位立方体中
    float center_x = (rect3d[0] + rect3d[3]) / 2;
    float center_y = (rect3d[1] + rect3d[4]) / 2;
    float center_z = (rect3d[2] + rect3d[5]) / 2;

    float max_length = rect3d[3] - rect3d[0];

    if(rect3d[4] - rect3d[1] > max_length)
    {
        max_length = rect3d[4] - rect3d[1];
    }
    if(rect3d[5] - rect3d[2] > max_length)
    {
        max_length = rect3d[5] - rect3d[2];
    }

    for(int i = 0; i < mesh->getVerticeSize(); ++i)
    {
        mesh->vertex_list[i].position_.setX((mesh->vertex_list[i].position_.x() - center_x) / max_length);
        mesh->vertex_list[i].position_.setY((mesh->vertex_list[i].position_.y() - center_y) / max_length);
        mesh->vertex_list[i].position_.setZ((mesh->vertex_list[i].position_.z() - center_z) / max_length);
    }

    mesh->updateArrayBuffer(mesh->vertex_list);

    return true;
}

std::vector<int> createMesh(QMaterial *material, Q3DScene *scene, QVector3D center, QVector3D eular, GLint *viewport)
{
    //申请新的mesh结构
    QCubeMesh *mesh = new QCubeMesh(material, scene);

    //mesh初始化顶点和面
    mesh->init();

    //mesh旋转平移
    mesh->setEular(eular);
    mesh->setOffset(center);

    //添加mesh到场景中
    scene->addModel(mesh);

    //设定标定rect初值
    int min_x = viewport[2] - 1;
    int max_x = 0;
    int min_y = viewport[3] - 1;
    int max_y = 0;

    //计算mesh对应世界坐标系的变换矩阵
    QMatrix4x4 transform_matrix;
    transform_matrix.setToIdentity();
    transform_matrix.translate(mesh->getOffset());

    QVector3D get_eular = mesh->getEular();
    transform_matrix.rotate(get_eular[0],QVector3D(1,0,0));
    transform_matrix.rotate(get_eular[1],QVector3D(0,1,0));
    transform_matrix.rotate(get_eular[2],QVector3D(0,0,1));

    //获取mesh的凸包上的点集
    std::vector<QVector3D> mesh_points;
    mesh_points.emplace_back(transform_matrix.map(QVector3D(-1, -1, -1)));
    mesh_points.emplace_back(transform_matrix.map(QVector3D(-1, -1,  1)));
    mesh_points.emplace_back(transform_matrix.map(QVector3D(-1,  1, -1)));
    mesh_points.emplace_back(transform_matrix.map(QVector3D(-1,  1,  1)));
    mesh_points.emplace_back(transform_matrix.map(QVector3D( 1, -1, -1)));
    mesh_points.emplace_back(transform_matrix.map(QVector3D( 1, -1,  1)));
    mesh_points.emplace_back(transform_matrix.map(QVector3D( 1,  1, -1)));
    mesh_points.emplace_back(transform_matrix.map(QVector3D( 1,  1,  1)));

    //逐个计算投影点并更新rect
    for(int i = 0; i < mesh_points.size(); ++i)
    {
        QVector3D win_pos = scene->project(mesh_points[i]);

        int win_x = viewport[0] + (win_pos[0] + 1) * viewport[2] / 2;
        int win_y = viewport[1] + (win_pos[1] + 1) * viewport[3] / 2;

        if(win_x < min_x)
        {
            min_x = win_x;
        }
        if(win_x > max_x)
        {
            max_x = win_x;
        }
        if(win_y < min_y)
        {
            min_y = win_y;
        }
        if(win_y > max_y)
        {
            max_y = win_y;
        }
    }

    //提高rect鲁棒性
    if(min_x < 0)
    {
        min_x = 0;
    }
    if(max_x >= viewport[2])
    {
        max_x = viewport[2] - 1;
    }
    if(min_y < 0)
    {
        min_y = 0;
    }
    if(max_y >= viewport[3])
    {
        max_y = viewport[3] - 1;
    }

    //存储rect为vector
    std::vector<int> rect;
    rect.emplace_back(min_x);
    rect.emplace_back(min_y);
    rect.emplace_back(max_x);
    rect.emplace_back(max_y);

    return rect;
}

QMesh3D *createMesh(QString mesh_name, QMaterial *material, Q3DScene *scene)
{
    //申请新的mesh结构
    QMesh3D *mesh = new QMesh3D(material, scene);

    //加载obj到mesh
    mesh->loadFile(mesh_name);

    //添加mesh到场景中
    scene->addModel(mesh);

    return mesh;
}

EasyMesh *createEasyMesh(QString mesh_name, QMaterial *material, Q3DScene *scene)
{
    //申请新的mesh结构
    EasyMesh *mesh = new EasyMesh(material, scene);

    //加载obj到mesh
    mesh->loadFile(mesh_name);

    //添加mesh到场景中
    scene->addModel(mesh);

    return mesh;
}

bool transformMesh(QMesh3D *mesh, QVector3D center, QVector3D eular)
{
    //mesh旋转
    mesh->rotateEular(eular);

    //mesh平移
    mesh->translate(center);

    return true;
}

bool transformEasyMesh(EasyMesh *mesh, QVector3D center, QVector3D eular)
{
    //mesh旋转
    mesh->rotateEular(eular);

    //mesh平移
    mesh->translate(center);

    return true;
}

bool setMeshpose(QMesh3D *mesh, QVector3D center, QVector3D eular)
{
    //mesh旋转
    mesh->setEular(eular);

    //mesh平移
    mesh->setOffset(center);

    return true;
}

bool setEasyMeshpose(EasyMesh *mesh, QVector3D center, QVector3D eular)
{
    //mesh旋转
    mesh->setEular(eular);

    //mesh平移
    mesh->setOffset(center);

    return true;
}

std::vector<int> getMeshProjectRect(QMesh3D *mesh, Q3DScene *scene, GLint *viewport)
{
    std::cout << mesh << std::endl;

    //设定标定rect初值
    int min_x = viewport[2] - 1;
    int max_x = 0;
    int min_y = viewport[3] - 1;
    int max_y = 0;

    //计算mesh对应世界坐标系的变换矩阵
    QMatrix4x4 transform_matrix;
    transform_matrix.setToIdentity();
    transform_matrix.translate(mesh->getOffset());

    QVector3D get_eular = mesh->getEular();
    transform_matrix.rotate(get_eular[0],QVector3D(1,0,0));
    transform_matrix.rotate(get_eular[1],QVector3D(0,1,0));
    transform_matrix.rotate(get_eular[2],QVector3D(0,0,1));

    std::cout << mesh->getVerticeSize() << std::endl;
    std::cout << mesh->getOffset().x() << "," << mesh->getOffset().y() << "," << mesh->getOffset().z() << std::endl;
    std::cout << get_eular[0] << "," << get_eular[1] << "," << get_eular[2] << std::endl << std::endl;

    //获取mesh的所有点集
    std::vector<QVector3D> mesh_points;

    QOpenGLBuffer *vertice_array = mesh->getVerticeArray();

    QMesh3D::VertexData *vertice_data = (QMesh3D::VertexData *)vertice_array->map(QOpenGLBuffer::ReadOnly);

    for(int i = 0; i < mesh->getVerticeSize(); ++i)
    {
        mesh_points.emplace_back(transform_matrix.map(vertice_data[i].position_));
    }

    vertice_array->unmap();

    //逐个计算投影点并更新rect
    for(int i = 0; i < mesh_points.size(); ++i)
    {
        QVector3D win_pos = scene->project(mesh_points[i]);

        int win_x = viewport[0] + (win_pos[0] + 1) * viewport[2] / 2;
        int win_y = viewport[1] + (win_pos[1] + 1) * viewport[3] / 2;

        if(win_x < min_x)
        {
            min_x = win_x;
        }
        if(win_x > max_x)
        {
            max_x = win_x;
        }
        if(win_y < min_y)
        {
            min_y = win_y;
        }
        if(win_y > max_y)
        {
            max_y = win_y;
        }
    }

    //提高rect鲁棒性
    if(min_x < 0)
    {
        min_x = 0;
    }
    if(max_x >= viewport[2])
    {
        max_x = viewport[2] - 1;
    }
    if(min_y < 0)
    {
        min_y = 0;
    }
    if(max_y >= viewport[3])
    {
        max_y = viewport[3] - 1;
    }

    //存储rect为vector
    std::vector<int> rect;
    rect.emplace_back(min_x);
    rect.emplace_back(min_y);
    rect.emplace_back(max_x);
    rect.emplace_back(max_y);

    return rect;
}

std::vector<int> getEasyMeshProjectRect(EasyMesh *mesh, Q3DScene *scene, GLint *viewport)
{
    std::cout << mesh << std::endl;

    //设定标定rect初值
    int min_x = viewport[2] - 1;
    int max_x = 0;
    int min_y = viewport[3] - 1;
    int max_y = 0;

    mesh->updateTransformMatrix();

    //获取mesh的所有点集
    std::vector<QVector3D> mesh_points;

    for(int i = 0; i < mesh->getVerticeSize(); ++i)
    {
        mesh_points.emplace_back(mesh->transform_matrix.map(mesh->vertex_list[i].position_));
    }

    //逐个计算投影点并更新rect
    for(int i = 0; i < mesh_points.size(); ++i)
    {
        QVector3D win_pos = scene->project(mesh_points[i]);

        int win_x = viewport[0] + (win_pos[0] + 1) * viewport[2] / 2;
        int win_y = viewport[1] + (win_pos[1] + 1) * viewport[3] / 2;

        if(win_x < min_x)
        {
            min_x = win_x;
        }
        if(win_x > max_x)
        {
            max_x = win_x;
        }
        if(win_y < min_y)
        {
            min_y = win_y;
        }
        if(win_y > max_y)
        {
            max_y = win_y;
        }
    }

    //提高rect鲁棒性
    if(min_x < 0)
    {
        min_x = 0;
    }
    if(max_x >= viewport[2])
    {
        max_x = viewport[2] - 1;
    }
    if(min_y < 0)
    {
        min_y = 0;
    }
    if(max_y >= viewport[3])
    {
        max_y = viewport[3] - 1;
    }

    //存储rect为vector
    std::vector<int> rect;
    rect.emplace_back(min_x);
    rect.emplace_back(min_y);
    rect.emplace_back(max_x);
    rect.emplace_back(max_y);

    return rect;
}

std::vector<std::vector<int>> getEasyMeshProjectRects(std::vector<EasyMesh *> mesh_list, Q3DScene *scene, GLint *viewport)
{
    std::vector<std::vector<int>> rect_list;

    for(int i = 0; i < mesh_list.size(); ++i)
    {
        rect_list.emplace_back(getEasyMeshProjectRect(mesh_list[i], scene, viewport));
    }

    return rect_list;
}

QMesh3D *getNewMeshRect3D(QString mesh_name, QMaterial *material, Q3DScene *scene, QVector3D center, QVector3D eular, GLint *viewport)
{
    QMesh3D *mesh = createMesh(mesh_name, material, scene);

    normalizeMesh(mesh);

    transformMesh(mesh, center, eular);

    return mesh;
}

EasyMesh *getNewEasyMeshRect3D(QString mesh_name, QMaterial *material, Q3DScene *scene, QVector3D center, QVector3D eular, GLint *viewport)
{
    EasyMesh *mesh = createEasyMesh(mesh_name, material, scene);

    normalizeEasyMesh(mesh);

    transformEasyMesh(mesh, center, eular);

    return mesh;
}

void saveImgAndJson(QString output_name, QRenderWidget &w, std::vector<std::vector<int>> rect_list, GLint *viewport)
{
    //抓取屏幕当前界面并保存
    QPixmap pixmap = w.grab();
    pixmap.save(output_name + ".jpg", "jpg");

    std::vector<int> rect_list_used_idx;
    for(int i = 0; i < rect_list.size(); ++i)
    {
        if(rect_list[i][0] < rect_list[i][2] && rect_list[i][1] < rect_list[i][3])
        {
            rect_list_used_idx.emplace_back(i);
        }
    }

    //创建labels序列
    QJsonArray json_labels;
    for(int i = 0; i < rect_list_used_idx.size(); ++i)
    {
        json_labels.append("0");
    }

    //创建polygons序列
    QJsonArray json_polygons;

    //创建polygon对应点序列并保存到polygons序列中
    for(int i = 0; i < rect_list_used_idx.size(); ++i)
    {
        QJsonArray polygonsArray;
        QJsonArray polygonArray;

        polygonArray.append(rect_list[rect_list_used_idx[i]][0]);
        polygonArray.append(viewport[3] - rect_list[rect_list_used_idx[i]][3]);
        polygonsArray.append(polygonArray);

        polygonArray[1] = viewport[3] - rect_list[rect_list_used_idx[i]][1];
        polygonsArray.append(polygonArray);

        polygonArray[0] = rect_list[rect_list_used_idx[i]][2];
        polygonsArray.append(polygonArray);

        polygonArray[1] = viewport[3] - rect_list[rect_list_used_idx[i]][3];
        polygonsArray.append(polygonArray);

        polygonArray[0] = rect_list[rect_list_used_idx[i]][0];
        polygonsArray.append(polygonArray);

        json_polygons.append(polygonsArray);
    }

    //创建shape序列
    QJsonArray json_shape;
    json_shape.append(viewport[2]);
    json_shape.append(viewport[3]);

    //创建Area字典并将前面所有内容载入
    QJsonObject json_Area;
    json_Area.insert("labels", QJsonValue(json_labels));
    json_Area.insert("polygons", QJsonValue(json_polygons));
    json_Area.insert("shape", QJsonValue(json_shape));

    //创建总字典并将Area字典载入
    QJsonObject json;
    json.insert("Area", QJsonValue(json_Area));

    //创建json文件并将前面创建的字典转为字符串
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);

    //保存json内容
    std::ofstream outfile(output_name.toStdString() + ".json");
    outfile << strJson.toStdString();

    outfile.close();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //配置OpenGL
    setGLFormat();

    //using namespace GCL;

    //创建主显示窗口
    QRenderWidget w;

    //显示
    w.show();

    //创建虚拟场景
    Q3DScene *scene = w.getScene();

    //创建Cube材质和纹理
    QMaterial *material = new QMaterial(scene);
    material->linkShaders(":/shaders/simple_vshader.glsl",":/shaders/simple_fshader.glsl");
    QImage image1("cube.png");
    material->addUniformTextureImage("texture",image1);

    //初始化相机位姿矩阵
    QMatrix4x4 matrix;
    matrix.lookAt(QVector3D(0,0,0),QVector3D(0,0,1),QVector3D(0,1,0));
    scene->setDefaultModelMatrix(matrix);
    scene->setDefaultView();

    //获取viewport参数
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    std::vector<EasyMesh *> easymesh_list;

    easymesh_list.emplace_back(getNewEasyMeshRect3D("c3.obj", material, scene, QVector3D( 4, 2, 10), QVector3D(rand()%360,rand()%360,rand()%360), viewport));
    easymesh_list.emplace_back(getNewEasyMeshRect3D("bunny.obj", material, scene, QVector3D( 0, 0, 10), QVector3D(rand()%360,rand()%360,rand()%360), viewport));
    easymesh_list.emplace_back(getNewEasyMeshRect3D("c3.obj", material, scene, QVector3D(-4, 2, 10), QVector3D(rand()%360,rand()%360,rand()%360), viewport));

    //保存抓取图片和对应json文件
    saveImgAndJson("out1", w, getEasyMeshProjectRects(easymesh_list, scene, viewport), viewport);

    //更新主窗口的显示
    w.show();

    return app.exec();

//    testOnScreen();
//    QDir dir("3waypipe");
//    QStringList filters;
//    filters<<"*.ply";
//    foreach (QFileInfo fileinfo, dir.entryInfoList(filters)) {
//        qDebug()<<fileinfo.absoluteFilePath();
//        testOffScreen(fileinfo.absoluteFilePath());
//    }


    testOnScreen();
//    testOffScreen("0000.ply");

//    using namespace GCL;
//    QOpenGLContext ctx;;
//    QSurfaceFormat surfaceFmt;
//    if(QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
//    {
//        surfaceFmt.setRenderableType(QSurfaceFormat::OpenGL);
//    }
//    else
//    {
//        surfaceFmt.setRenderableType(QSurfaceFormat::OpenGLES);
//        //surfaceFmt.setVersion(3,0);
//    }
//    ctx.setFormat(surfaceFmt);
//    bool b = ctx.create();
//    surfaceFmt.setDepthBufferSize(24);
//    QSurfaceFormat::setDefaultFormat(surfaceFmt);

//    QOffscreenSurface* pSurface = new QOffscreenSurface;
//    pSurface->setFormat(surfaceFmt);
//    pSurface->create();

//    ctx.makeCurrent(pSurface);


//    Q3DScene scene;


//    scene.init();

//   QMaterial *material = new QMaterial(&scene);
//    material->linkShaders(":/shaders/simple_vshader.glsl",":/shaders/simple_fshader.glsl");
//    QImage image1("cube.png");
//    material->addUniformTextureImage("texture",image1);
//    scene.addModel(new QCubeMesh(material, &scene));
//    QMatrix4x4 matrix;
//    matrix.lookAt(QVector3D(0,0,0),QVector3D(0,0,1),QVector3D(0,1,0));
//    matrix.translate(-0,0,10);

//    scene.setDefaultModelMatrix(matrix);
//    scene.setDefaultView();
//    int w = 800;
//    int h = 800;
//    QOpenGLFramebufferObject fbo(QSize(w,h),QOpenGLFramebufferObject::Depth);
//    scene.resize(w,h);

//    scene.manipulator_rotate(0,10);
//    fbo.bind();
//    scene.render();
//    fbo.bindDefault();
//    QImage image = fbo.toImage();
//    image.save("test.png");



    return 0;
}
