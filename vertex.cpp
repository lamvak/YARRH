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
    return *this;
}

Vertex& Vertex::operator=(const QVector3D & other){
    this->setX(other.x());
    this->setY(other.y());
    this->setZ(other.z());
    return *this;
}
