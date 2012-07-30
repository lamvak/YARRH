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
    QMap<QString,HalfEdge*> edges;
    //list of faces
    QHash<QString,Face*> faces;
    //here is list of edges connecting only one face E(p1,p2)==E(p2,p1)
    QList<QString> badEdges;
    //function for adding verteces
    Vertex* addVertex(float x, float y, float z);
    //function for adding haledges
    HalfEdge* addHalfEdge(Vertex *v1, Vertex *v2);
    //function for adding faces
    Face* addFace(HalfEdge *edge1, HalfEdge *edge2, HalfEdge *edge3);
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
