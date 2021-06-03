#include "QMeshParser.h"
#include <QFileInfo>
#include <QVector3D>
#include <QVector4D>
#include <QVector2D>

#include <iostream>
namespace GCL {

QTriMeshParser::QTriMeshParser()
{

}

void QTriMeshParser::loadFile(const QString &filename, std::vector<QMesh3D::VertexData> &vertices, std::vector<QMesh3D::FaceData> &facedata, bool flat)
{
    QFileInfo fileinfo(filename);
    if(fileinfo.suffix().toLower() == "obj")
    {
        loadObjFile(filename,vertices,facedata,flat);
    }
}
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));

    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}
void QTriMeshParser::loadObjFile(const QString &filename, std::vector<QMesh3D::VertexData> &vertices, std::vector<QMesh3D::FaceData> &facedata, bool flat)
{
    QFile file(filename);
    if(!file.open(QFile::ReadOnly)) {
        qDebug()<<"open "<<filename<<" failed!";
        return;
    }
    using Vec3 = QVector3D;
    using Vec4 = QVector4D;
    using Vec2 = QVector2D;
    using Vec3i = QVector3D;

    const int maxSize = 512;
    char *pLine;
    std::vector<Vec3> pos_list;
    std::vector<Vec4> color_list_;
    std::vector<Vec3i> face_list;
    std::vector<Vec3i> face_texture_list;
    std::vector< Vec2 > texcordList;
    QByteArray bytes = file.readAll();
    QByteArrayList list = bytes.split('\n');
    for(auto &line : list)
    {
        pLine = line.data();
        if(pLine[0] == 'v' && pLine[1] == ' ')
        {
            Vec3 pos;
            Vec4 color(0.0,0.0,0.0,0);
            char* tok =strtok(pLine," ");
            for(int i=0; i < 7; i++)
            {
                tok = strtok(NULL," ");
                if(!tok) break;
                if(i < 3)
                {
                    pos[i] = (qreal)atof(tok);
                }
                else
                {
                    color[i-3] = (qreal)atof(tok);
                }
            }
            pos_list.push_back(pos);
            color_list_.push_back(color);

        }
        else if(pLine[0] == 'v' && pLine[1] == 't')
        {
            Vec2 tex;
            char* tok  = strtok(pLine," ");
            for(int i=0; i < 2; i++)
            {
                tok = strtok(NULL," ");
                tex[i] = (qreal)atof(tok);
            }
            texcordList.push_back(tex);
        }
        else if(pLine[0] == 'f' && pLine[1] == ' ')
        {
          //  char* tok = strtok(pLine," ");
            Vec3i f;
            std::vector<std::string> flist;
            std::vector<int> fi_list;
            std::vector<int> fti_list;
            SplitString(pLine,flist," ");
            if(flist.size() < 4) continue;
            for(int i=1; i < flist.size(); i++)
            {
                std::vector<std::string> f_t;
                SplitString(flist[i],f_t,"/");
                int f0 = strtol(f_t[0].c_str(),NULL,10)-1;
                fi_list.push_back(f0);
                if(f_t.size() >= 2)
                {
                    int f1 = strtol(f_t[1].c_str(),NULL,10)-1;
                    fti_list.push_back(f1);
                }
            }
            for(int i=1; i < fi_list.size()-1; i++)
            {
                Vec3i f;
                f[0] = fi_list[i];
                f[1] = fi_list[i+1];
                f[2] = fi_list[0];
                face_list.push_back(f);
                if(fti_list.size())
                {
                    Vec3i ft;
                    ft[0] = fti_list[i];
                    ft[1] = fti_list[i+1];
                    ft[2] = fti_list[0];
                    face_texture_list.push_back(ft);
                }

            }
        }
    }
    file.close();
//    std::cout<<pos_list.size()<<" "<<face_list.size()<<std::endl;
    vertices.clear();

    if(flat)
    {
        for(int i=0; i < pos_list.size(); i++)
        {
            vertices.push_back(QMesh3D::VertexData(pos_list[i]));
        }
        std::vector<QMesh3D::VertexData> res_data;
        for(int i=0; i <face_list.size(); i++)
        {
            Vec3i f = face_list[i];

            QMesh3D::VertexData v0 = pos_list[f[0]];
            QMesh3D::VertexData v1 = pos_list[f[1]];
            QMesh3D::VertexData v2 = vertices[f[2]];

            Vec3 p0 = v0.position_;
            Vec3 p1 = v1.position_ - p0;
            Vec3 p2 = v2.position_ - p0;
            Vec3 nor = Vec3::crossProduct(p1,p2).normalized();

            v0.normal_ = nor;
            v1.normal_ = nor;
            v2.normal_ = nor;

            res_data.push_back(v0);
            res_data.push_back(v1);
            res_data.push_back(v2);

            facedata.push_back(3 * i);
            facedata.push_back(3 * i+1);
            facedata.push_back(3 * i + 2);
        }

        vertices = res_data;
    }
    else
    {
        for(int i=0; i < pos_list.size(); i++)
        {
            vertices.push_back(QMesh3D::VertexData(pos_list[i]));
        }
        for(int i=0; i < face_list.size(); i++)
        {
            Vec3i f = face_list[i];

             facedata.push_back(f[0]);
             facedata.push_back(f[1]);
             facedata.push_back(f[2]);

             Vec3 p0 = vertices[f[0]].position_;
             Vec3 p1 = vertices[f[1]].position_ - p0;
             Vec3 p2 = vertices[f[2]].position_ - p0;

             Vec3 nor = Vec3::crossProduct(p1,p2).normalized();

             vertices[f[0]].normal_ += nor;
             vertices[f[1]].normal_ += nor;
             vertices[f[2]].normal_ += nor;
        }
        for(auto &v :vertices)
        {
            v.normal_.normalize();
        }
    }

//    std::cout<<vertices.size()<<std::endl;

}
}
