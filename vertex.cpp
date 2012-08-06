#include "vertex.h"

Vertex::Vertex(Vertex *parent)
{
}

Vertex::Vertex(qreal xpos, qreal ypos, qreal zpos):QVector3D(xpos,ypos,zpos){

}

Vertex& Vertex::operator=(const Vertex & other){
    this->setX(other.x());
    this->setY(other.y());
    this->setZ(other.z());
    this->edges=other.edges;
    return *this;
}

Vertex& Vertex::operator=(const QVector3D & other){
    this->setX(other.x());
    this->setY(other.y());
    this->setZ(other.z());
    this->edges.clear();
    return *this;
}

void Vertex::addHalEdge(HalfEdge *edge){
    if(edge!=NULL && !this->edges.contains(edge)){
        this->edges.append(edge);
    }
}

void Vertex::removeHalEdge(HalfEdge *edge){
    this->edges.removeAt(this->edges.indexOf(edge));
}

QList<HalfEdge*> Vertex::getEdges(){
    return this->edges;
}

QVector3D Vertex::toVector3D(){
    return QVector3D(this->x(),this->y(),this->z());
}
