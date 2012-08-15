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
    first=QString::number((size_t)start);
    second=QString::number((size_t)stop);
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

double HalfEdge::angleBeetwen(HalfEdge *second){
    QVector3D vertex1=this->start->toVector3D()*100;
    QVector3D vertex2=this->stop->toVector3D()*100;;
    QVector3D point1;
    QVector3D vertex3=second->start->toVector3D()*100;;
    QVector3D vertex4=second->stop->toVector3D()*100;;
    QVector3D point2;
    //move verteces to point 0,0
    qDebug() << "Vertex1" << vertex1.x() << vertex1.y() << vertex1.z();
    qDebug() << "Vertex2" << vertex2.x() << vertex2.y() << vertex2.z();
    qDebug() << "Vertex3" << vertex3.x() << vertex3.y() << vertex3.z();
    qDebug() << "Vertex4" << vertex4.x() << vertex4.y() << vertex4.z();

    if(vertex1.length()>vertex2.length()){
        point1=vertex1-vertex2;
    }
    else{
        point1=vertex2-vertex1;
    }
    qDebug() << vertex1.length() << vertex2.length();
    qDebug() << point1.x() << point1.y() << point1.z();

    if(vertex3.length()>vertex4.length()){
        point2=vertex3-vertex4;
    }
    else{
        point2=vertex4-vertex3;
    }
    qDebug() << vertex3.length() << vertex4.length();
    qDebug() << point2.x() << point2.y() << point2.z();
    double dotProduct=QVector3D::dotProduct(point1.normalized(),point2.normalized());
    double angle=(acos(dotProduct)*(double)180)/3.14159;
    return angle;
}
