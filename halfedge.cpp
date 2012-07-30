#include "halfedge.h"

//edges
HalfEdge::HalfEdge(Vertex* start, Vertex* stop){
    this->start=start;
    this->stop=stop;
    this->hash=computeHash();
    this->twin=NULL;
}

HalfEdge::HalfEdge(): start(NULL), stop(NULL){
}

QString HalfEdge::computeHash(){
    QString first;
    QString second;
    first=QString::number((int)start);
    second=QString::number((int)stop);
    first.append(second);
    return first;
}

QString HalfEdge::getHash(){
    return this->hash;
}


void HalfEdge::addFace(Face *face){
    if(!this->faces.contains(face)){
        this->faces.append(face);
    }
}

QList<Face*> HalfEdge::getFaces(){
    return this->faces;
}

void HalfEdge::setTwin(HalfEdge *twin){
    this->twin=twin;
}

void HalfEdge::removeFace(Face* face){
    this->faces.removeAt(this->faces.indexOf(face));
}
