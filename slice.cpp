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
    //create list
    for(int j=0; j<layersNum+1; j++){
        this->triLayer.append(new QList<Face*>);
        this->edgeLayer.append(new QList< QList<HalfEdge*> *>);
        this->pointLayer.append(new QList< QList<QVector3D> *>);
    }
    double tempZ;
    for(i = faces.begin(); i != faces.end(); ++i){
        tempZ=(int)((i.value()->getMinZ()->z()/layerHeight)+1)*layerHeight;
        while(tempZ<i.value()->getMaxZ()->z()){
            this->triLayer.at((int)ceil(tempZ/layerHeight))->append(i.value());
            tempZ+=this->layerHeight;
        }
    }
    fillEdgeLayer();
    fillPointLayer();
    //fillPolygonLayer();
}

//fill list of edges crossing plane on some z height
void Slice::fillEdgeLayer(){
    QList<Face*> currentLayer;
    Face* currentFace;
    HalfEdge* currentEdge;
    for(int i=0; i<this->triLayer.size(); i++){
        currentLayer=*this->triLayer.at(i);
        //take first face from list
        if(currentLayer.size()>0){
            currentFace=currentLayer.takeFirst();
            currentEdge=currentFace->getEdgesCrossingPlane(this->layerHeight*i).at(0);
            //create list for firts edge loop
            this->edgeLayer.at(i)->append(new QList<HalfEdge*>);
            this->pointLayer.at(i)->append(new QList<QVector3D>);
            this->edgeLayer.at(i)->last()->append(currentEdge);
            //while all faces are processed
            while(currentLayer.size()>0){
//                //take his twin
                currentEdge=currentEdge->getTwin();
                if(currentEdge->getFaces().size()>0){
                    currentFace=currentEdge->getFaces().at(0);
                    currentEdge=currentFace->getEdgesCrossingPlane(this->layerHeight*i,currentEdge).first();
                    this->edgeLayer.at(i)->last()->append(currentEdge);
                }
                if(currentLayer.contains(currentFace)){
                    currentLayer.removeAt(currentLayer.indexOf(currentFace));
                }
                else{
                    if(!currentLayer.isEmpty()){
                       currentFace=currentLayer.takeFirst();
                       currentEdge=currentFace->getEdgesCrossingPlane(this->layerHeight*i).at(0);
                       this->edgeLayer.at(i)->append(new QList<HalfEdge*>);
                       this->pointLayer.at(i)->append(new QList<QVector3D>);
                    }
                }
            }
        }
    }
}
//find point of polygon
void Slice::fillPointLayer(){
    //for each layer
    for(int i=0; i<this->edgeLayer.size(); i++){
        //and each loop
        for(int j=0; j<this->edgeLayer.at(i)->size(); j++){
            for(int k=0; k<this->edgeLayer.at(i)->at(j)->size(); k++){
                this->pointLayer.at(i)->at(j)->append(linePLaneIntersection(*this->edgeLayer.at(i)->at(j)->at(k),QVector3D(0,0,this->layerHeight*i), QVector3D(0,0,1)));
            }
        }
    }
}

void Slice::fillPolygonLayer(){
//    ClipperLib::Polygon p;
//    qDebug() << "loop size"<< this->pointLayer.at(1)->at(0)->size();
//    for(int i=0; i<this->pointLayer.at(10)->at(0)->size(); i++){
//        p.push_back(ClipperLib::IntPoint((int)(this->pointLayer.at(10)->at(0)->at(i).x()*1000),(int)(this->pointLayer.at(10)->at(0)->at(i).y()*1000)));
//        GLdouble * newVertex;
//        newVertex = (GLdouble *)malloc(3*sizeof*newVertex);
//        newVertex[0] = this->pointLayer.at(10)->at(0)->at(i).x();
//        newVertex[1] = this->pointLayer.at(10)->at(0)->at(i).y();
//        newVertex[2] = this->pointLayer.at(10)->at(0)->at(i).z();
//        this->vectors.append(newVertex);
//    }
//    Polygons layer;
//    layer.push_back(p);
//    this->polygonLayer.append(layer);
}

QVector3D Slice::linePLaneIntersection(HalfEdge edge, QVector3D planeOrigin, QVector3D planeNormal){
    QVector3D u=edge.getStart()->toVector3D()-edge.getStop()->toVector3D();
    QVector3D w=edge.getStop()->toVector3D()-planeOrigin;
    QVector3D normal=planeNormal;

    float d=QVector3D::dotProduct(normal,u);
    float n=-QVector3D::dotProduct(normal,w);
    //line is parallel to plane
    if(fabs(d)<0.00000001){
        if(n==0){
            qDebug() << "line is on plane";
            return QVector3D();
        }
        else{
            qDebug() << "no intersection";
            return QVector3D();
        }
    }
    //line croses plane
    float sI = n/d;
    if(sI<0 || sI>1){
        qDebug() << "no intersection";
        return QVector3D();
    }
    return edge.getStop()->toVector3D() + u*sI;
}

void Slice::draw(int layer){
    GLfloat   green[4]={0.0,1.0,0.0,1.0};
    GLfloat   blue[4]={0.0,0.0,1.0,1.0};
    GLfloat   fCurrentColor[4];
    // Get the current color
    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);
    if(layer<this->triLayer.size()){
//        //draw faces
//        for(int i=0; i<this->triLayer.at(layer)->size(); i++){
//            glBegin(GL_TRIANGLES);
//            glNormal3f(this->triLayer.at(layer)->at(i)->getNormal()->x(),this->triLayer.at(layer)->at(i)->getNormal()->y(),this->triLayer.at(layer)->at(i)->getNormal()->z());
//            glVertex3f(this->triLayer.at(layer)->at(i)->getEdge1()->getStart()->x(), this->triLayer.at(layer)->at(i)->getEdge1()->getStart()->y(), this->triLayer.at(layer)->at(i)->getEdge1()->getStart()->z());
//            glVertex3f(this->triLayer.at(layer)->at(i)->getEdge2()->getStart()->x(), this->triLayer.at(layer)->at(i)->getEdge2()->getStart()->y(), this->triLayer.at(layer)->at(i)->getEdge2()->getStart()->z());
//            glVertex3f(this->triLayer.at(layer)->at(i)->getEdge3()->getStart()->x(), this->triLayer.at(layer)->at(i)->getEdge3()->getStart()->y(), this->triLayer.at(layer)->at(i)->getEdge3()->getStart()->z());
//            glEnd( );
//        }
//        //draw edges
        glDisable(GL_LIGHTING);
//        glColor4fv(green);
//        glLineWidth(1.0);
        glDepthMask(false);
//        glBegin(GL_LINES);
//        for(int i=0; i<this->edgeLayer.at(layer)->size(); i++){
//            for(int j=0; j<this->edgeLayer.at(layer)->at(i)->size(); j++){
//                glVertex3f(this->edgeLayer.at(layer)->at(i)->at(j)->getStart()->x(), this->edgeLayer.at(layer)->at(i)->at(j)->getStart()->y(), this->edgeLayer.at(layer)->at(i)->at(j)->getStart()->z());
//                glVertex3f(this->edgeLayer.at(layer)->at(i)->at(j)->getStop()->x(), this->edgeLayer.at(layer)->at(i)->at(j)->getStop()->y(), this->edgeLayer.at(layer)->at(i)->at(j)->getStop()->z());
//            }
//        }
//        glEnd( );
        //draw intersection points
        //glColor4fv(blue);
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        for(int i=0; i<this->pointLayer.at(layer)->size(); i++){
            for(int j=0; j<this->pointLayer.at(layer)->at(i)->size(); j++){
                qDebug() << (float)j/(float)this->pointLayer.at(layer)->at(i)->size();
                glColor4f(0.0,(float)j/(float)this->pointLayer.at(layer)->at(i)->size(),0.0,1.0);
                glVertex3f(this->pointLayer.at(layer)->at(i)->at(j).x(), this->pointLayer.at(layer)->at(i)->at(j).y(), this->pointLayer.at(layer)->at(i)->at(j).z() );
            }
        }
        glEnd();
        glDepthMask(true);
        glEnable(GL_LIGHTING);
       // drawPolygon(layer);
    }
    glColor4fv(fCurrentColor);
}

void Slice::drawPolygon(int layer){

    for(int i=0; i<this->pointLayer.at(layer)->size(); i++){
        glBegin(GL_POLYGON);
        for(int j=0; j<this->pointLayer.at(layer)->at(i)->size(); j++){
            glVertex3f(this->pointLayer.at(layer)->at(i)->at(j).x(), this->pointLayer.at(layer)->at(i)->at(j).y(), this->pointLayer.at(layer)->at(i)->at(j).z() );
        }
        glEnd();
    }
}
