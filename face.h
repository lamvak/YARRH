#ifndef FACE_H
#define FACE_H
#include <QVector3D>
#include <QDebug>
#include "halfedge.h"
#include "vertex.h"



//forward declaration
class HalfEdge;
class Vertex;
//class for faces
class Face{
public:
    explicit Face();
    explicit Face(HalfEdge *e1, HalfEdge *e2, HalfEdge *e3);
    //changes winding of face so normal vector is calculated
    void flip();
    inline QVector3D* getNormal(){return normal;}
    //change to list or vector... some day some time
    inline HalfEdge* getEdge1(){return edge1;}
    inline HalfEdge* getEdge2(){return edge2;}
    inline HalfEdge* getEdge3(){return edge3;}
    QList< HalfEdge* > getEdgesCrossingPlane(double z, HalfEdge* remove=NULL );
    inline Vertex* getMaxZ() {return maxZ;}
    inline Vertex* getMinZ() {return minZ;}
    inline bool isNormalGood() {return goodNormal;}
    QList<Face*> getNeighbors();
    QVector3D getCenter();
    QVector3D* computeNormal(QVector3D* p1, QVector3D* p2, QVector3D* p3);
    void setNormalIsGood(bool value);
    QString getHash();
    inline qreal getAngle(){return angle;}
private:
    HalfEdge* edge1;
    HalfEdge* edge2;
    HalfEdge* edge3;
    Vertex* maxZ;
    Vertex* minZ;
    QVector3D* normal;
    qreal angle;
    QList<Face*> neighbors;
    bool goodNormal;
};

#endif // FACE_H
