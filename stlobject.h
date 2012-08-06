#ifndef STLOBJECT_H
#define STLOBJECT_H

#include <QObject>
#include <QtOpenGL>
//#include <GL/glut.h>
#include <GL/gl.h>
#include <QVector3D>
#include <QHash>
#include <QCryptographicHash>
#include <QDebug>
#include <cmath>
//half edges
#include "vertex.h"
#include "halfedge.h"
#include "face.h"


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
private:
    double scaleValue;
    double rotation;
    QString nameString;
    QString fileName;
    QVector3D offset;
    bool selected;
    bool colliding;
    float width;
    float lenght;
    float height;
    bool nonManifold;
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
    Vertex* addVertex(float x, float y, float z);
    //function for adding haledges
    HalfEdge* addHalfEdge(Vertex *v1, Vertex *v2);
    //function for adding faces
    Face* addFace(HalfEdge *edge1, HalfEdge *edge2, HalfEdge *edge3);
    //just a helper function
    QList<HalfEdge*> intersectLists(QList<HalfEdge*> list1, QList<HalfEdge*> list2);

    //calculating angle beetwen two QVector3D
    double angle(QVector3D p1, QVector3D p2);
public:
    explicit StlObject(QString fileName, QObject *parent = 0);
    explicit StlObject(QObject *parent = 0);
    void draw(bool);
    void select(bool);
    void moveXY(double x, double y);
    void rotate(double angle);
    void scale(double value);
    void loadFile(QString fileName);
    QVector3D getOffset();
    QList<QVector3D> getTriangles();
    double getRotation();
    double getScale();
    inline bool getIsManifold(){return nonManifold;}
    void mirror(QChar axis);
    void repairHoles();
    void repairNormals();
signals:
    void progress(int value, int max, QString text);
    void doneProcessing(bool);
public slots:

};

#endif // STLOBJECT_H
