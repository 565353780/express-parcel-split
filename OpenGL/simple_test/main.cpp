
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
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    setGLFormat();

    using namespace GCL;

    QRenderWidget w;
    w.show();
    Q3DScene *scene = w.getScene();


   QMaterial *material = new QMaterial(scene);
    material->linkShaders(":/shaders/simple_vshader.glsl",":/shaders/simple_fshader.glsl");
    QImage image1("cube.png");
    material->addUniformTextureImage("texture",image1);
    scene->addModel(new QCubeMesh(material, scene));
    QMatrix4x4 matrix;
    matrix.lookAt(QVector3D(0,0,0),QVector3D(0,0,1),QVector3D(0,1,0));
    matrix.translate(-0,0,10);

    scene->setDefaultModelMatrix(matrix);
    scene->setDefaultView();
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


//    testOnScreen();
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



//    return 0;
}
