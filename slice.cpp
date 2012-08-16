#include "slice.h"

Slice::Slice(QObject *parent) :
    QObject(parent)
{
}


void Slice::fillTriLayer(QHash<QString, Face*> faces){
    QHash<QString, Face*>::iterator i;
    double zMax=0;
    for(i = faces.begin(); i != faces.end(); ++i){
        if(zMax<i.value()->getMaxZ()->z())
            zMax=i.value()->getMaxZ()->z();
    }
    int layersNum=(int)ceil(zMax/layerHeight);
    qDebug() << "Max z: "<< zMax << "Number of layers: "<<layersNum;
    for(int j=0; j<layersNum+1; j++){
        this->triLayer.append(new QList<Face*>);
    }
    double tempZ;
    for(i = faces.begin(); i != faces.end(); ++i){
        tempZ=i.value()->getMinZ()->z();
        while(tempZ<i.value()->getMaxZ()->z()){
            this->triLayer.at((int)ceil(tempZ/layerHeight))->append(i.value());
            tempZ+=this->layerHeight;
        }
    }
}

void Slice::draw(int layer){

    if(layer<this->triLayer.size()){
        for(int i=0; i<this->triLayer.at(layer)->size(); i++){
            glBegin(GL_TRIANGLES);
            glNormal3f(this->triLayer.at(layer)->at(i)->getNormal()->x(),this->triLayer.at(layer)->at(i)->getNormal()->y(),this->triLayer.at(layer)->at(i)->getNormal()->z());
            glVertex3f(this->triLayer.at(layer)->at(i)->getEdge1()->getStart()->x(), this->triLayer.at(layer)->at(i)->getEdge1()->getStart()->y(), this->triLayer.at(layer)->at(i)->getEdge1()->getStart()->z());
            glVertex3f(this->triLayer.at(layer)->at(i)->getEdge2()->getStart()->x(), this->triLayer.at(layer)->at(i)->getEdge2()->getStart()->y(), this->triLayer.at(layer)->at(i)->getEdge2()->getStart()->z());
            glVertex3f(this->triLayer.at(layer)->at(i)->getEdge3()->getStart()->x(), this->triLayer.at(layer)->at(i)->getEdge3()->getStart()->y(), this->triLayer.at(layer)->at(i)->getEdge3()->getStart()->z());
            glEnd( );
        }
    }
}
