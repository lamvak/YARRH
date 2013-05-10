#ifndef STLOBJECT_H
#define STLOBJECT_H

#include <QObject>
#include <QtOpenGL>

#if defined(Q_WS_MAC)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined(Q_WS_WIN)
#include <GL/gl.h>
#include <GL/glu.h>
#elif defined(Q_WS_X11)
#include <GLES/gl2.h>
#endif
#include <QVector3D>
#include <QHash>
#include <QCryptographicHash>
#include <QDebug>
#include <cmath>
//half edges
#include "vertex.h"
#include "halfedge.h"
#include "face.h"
//materials
#include "material.h"
//slicer
#include "slice.h"


//for sorting q list by angles
struct QPairFirstComparer
{
    template<typename T1, typename T2>
    bool operator()(const QPair<T1,T2> & a, const QPair<T1,T2> & b) const
    {
        return a.first < b.first;
    }
};

//structure for holding data about created patch face
struct PatchFace
{
   Vertex* middlePoint;
   HalfEdge* edge1;
   HalfEdge* edge2;
   double angle;
};

class StlObject : public QObject
{
    Q_OBJECT
protected:
    QString fileName;
private:
    double scaleValue;
    double rotation;
    QString nameString;
    QVector3D offset;
    bool selected;
    bool colliding;
    float width;
    float lenght;
    float height;
    bool nonManifold;
    bool mirrorX;
    bool mirrorY;
    bool mirrorZ;
    bool show_angles;
    bool show_support;
    int list_index;
    int angle_list_index;
    int support_list_index;
    bool preSelected;
    Material* objectMaterial;
    //object description

    //list of verteces
    QHash<QString,Vertex*> vertexes;
    //list of edges
    QHash<QString,HalfEdge*> edges;
    //list of faces
    QHash<QString,Face*> faces;
    //here is list of edges connecting only one face E(p1,p2)==E(p2,p1)
    QList<HalfEdge*> badEdges;
    //function for adding verteces
    Vertex* addVertex(float x, float y, float z, QHash<QString,Vertex*> &list);
    //function for adding haledges
    HalfEdge* addHalfEdge(Vertex *v1, Vertex *v2,QHash<QString,HalfEdge*> &list);
    //function for adding faces
    Face* addFace(HalfEdge *edge1, HalfEdge *edge2, HalfEdge *edge3, QHash<QString,Face*> &list);
    //just a helper function
    QList<HalfEdge*> intersectLists(QList<HalfEdge*> list1, QList<HalfEdge*> list2);
    //make gl list
    void render();
    //calculating angle beetwen two QVector3D
    double angle(QVector3D p1, QVector3D p2);
    //slicer object, it stores info about each of layers
    //Slice* slicer;
    //things realeated to genereting support
    //list of verteces
    QHash<QString,Vertex*> support_vertexes;
    //list of edges
    QHash<QString,HalfEdge*> support_edges;
    //list of faces
    QHash<QString,Face*> support_faces;
public:

    explicit StlObject(QString fileName, QObject *parent = 0);
    explicit StlObject(QObject *parent = 0);
    inline QHash<QString,Face*> getFaces() {return faces;}
    void copy(StlObject *copyFrom);
    void draw(bool,bool,int);
    void select(bool);
    void moveXY(double x, double y);
    void rotate(double angle);
    void setRotation(double angle);
    void scale(double value);
    void loadFile(QString fileName);
    void set_showAngles(bool show);
    void set_showSupport(bool show);
    QVector3D getOffset();
    QList<QVector3D> getTriangles();
    double getRotation();
    double getScale();
    inline bool getIsManifold(){return nonManifold;}
    void mirror(QChar axis);
    void repairHoles();
    void repairNormals();
    inline void setMaterial(Material* material){ objectMaterial=material ;}
    inline Material* getMaterial() { return objectMaterial; }
    QString getFileName();
    inline double getWidth(){ return width*scaleValue; }
    inline double getLenght(){ return lenght*scaleValue; }
    inline double getHeight(){ return height*scaleValue; }
    inline bool isMirrored() { return mirrorX^mirrorY; }
    inline bool isSelected() { return selected; }
    inline bool get_showAngles() {return show_angles;}
    inline bool get_showSupport() {return show_support;}
    void freeList();

    void generateSupport(qreal treshold);
signals:
    void progress(int value, int max, QString text);
    void doneProcessing(bool);
public slots:

};

#endif // STLOBJECT_H
