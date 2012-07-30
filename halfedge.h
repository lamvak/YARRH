#ifndef HALFEDGE_H
#define HALFEDGE_H
#include <QVector3D>
#include <QString>
#include "face.h"
#include "vertex.h"

class Face;
//class for halfedges
class HalfEdge{
public:
    HalfEdge();
    HalfEdge(Vertex* start, Vertex* stop);
    inline Vertex* getStart(){ return start; }
    inline Vertex* getStop(){ return stop; }
    bool operator== ( const HalfEdge & rhs)
    {
         return start == rhs.start && stop ==rhs.stop;
    }
    QString computeHash();
    QString getHash();
    QList<Face*> getFaces();
    inline HalfEdge* getTwin(){ return twin; }
    void addFace(Face* face);
    void removeFace(Face* face);
    void setTwin(HalfEdge* twin);
private:
    //twin of this edge
    HalfEdge *twin;
    //face using this halfedge, if is null then there is something wrong with mesh [non manifold or bad normals]
    QList<Face*> faces;
    Vertex* start;
    Vertex* stop;
    QString hash;
};

#endif // HALFEDGE_H
