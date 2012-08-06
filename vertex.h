#ifndef VERTEX_H
#define VERTEX_H

#include <QVector3D>
#include "halfedge.h"

class HalfEdge;

class Vertex : public QObject, public QVector3D
{
    Q_OBJECT
private:
    QList<HalfEdge*> edges;
public:
    explicit Vertex(Vertex *parent = 0);
    explicit Vertex(qreal xpos, qreal ypos, qreal zpos);
    Vertex & operator=(const Vertex & other);
    Vertex & operator=(const QVector3D & other);
    void addHalEdge(HalfEdge *edge);
    void removeHalEdge(HalfEdge *edge);
    QList<HalfEdge*> getEdges();
    QVector3D toVector3D();
signals:

public slots:

};

#endif // VERTEX_H
