#include "stlobject.h"

//stl object
StlObject::StlObject(QObject *parent) :
QObject(parent)
{
    this->scaleValue=1.0;
    this->rotation=0.0;
    this->selected=false;
    this->colliding=false;
    this->nonManifold=false;
}

StlObject::StlObject(QString fileName, QObject *parent) :
    QObject(parent)
{
    this->scaleValue=1.0;
    this->rotation=0.0;
    this->selected=false;
    this->colliding=false;
    this->nonManifold=false;
    this->loadFile(fileName);
}

void StlObject::loadFile(QString fileName){
    QTime myTimer;
    bool badNormals;
    myTimer.start();
    this->fileName=fileName;
    QFile* file = new QFile(fileName);
    if(file->open(QIODevice::ReadOnly)){
        float xMax=0,xMin=0;
        float yMax=0,yMin=0;
        float zMax=0,zMin=0;
        int triang_count = 0;

        Vertex *v1;
        Vertex *v2;
        Vertex *v3;
        HalfEdge *e1;
        HalfEdge *e2;
        HalfEdge *e3;
        Face *face;
        float nx, ny, nz, x1, x2, x3, y1, y2, y3, z1, z2, z3;
        QString line;
        line=file->readLine();
        line.remove("\r");
        //ascii file
        if(line.contains("solid")){
            while(!file->atEnd()){
                line=file->readLine();
                line.remove("\r");
                if(line.contains("endsolid")){
                    break;
                }
                nx=line.split(" ", QString::SkipEmptyParts).at(2).toFloat();
                ny=line.split(" ", QString::SkipEmptyParts).at(3).toFloat();
                nz=line.split(" ", QString::SkipEmptyParts).at(4).toFloat();
                line=file->readLine();
                line.remove("\r");
                line=file->readLine();
                line.remove("\r");
                x1=line.split(" ",QString::SkipEmptyParts).at(1).toFloat();
                y1=line.split(" ",QString::SkipEmptyParts).at(2).toFloat();
                z1=line.split(" ",QString::SkipEmptyParts).at(3).toFloat();
                line=file->readLine();
                line.remove("\r");
                x2=line.split(" ",QString::SkipEmptyParts).at(1).toFloat();
                y2=line.split(" ",QString::SkipEmptyParts).at(2).toFloat();
                z2=line.split(" ",QString::SkipEmptyParts).at(3).toFloat();
                line=file->readLine();
                line.remove("\r");
                x3=line.split(" ",QString::SkipEmptyParts).at(1).toFloat();
                y3=line.split(" ",QString::SkipEmptyParts).at(2).toFloat();
                z3=line.split(" ",QString::SkipEmptyParts).at(3).toFloat();
                line=file->readLine();
                line.remove("\r");
                line=file->readLine();
                line.remove("\r");
                if(triang_count==0){
                    xMax=x1;
                    yMax=y1;
                    zMax=z1;
                    xMin=x1;
                    yMin=y1;
                    zMin=z1;
                }

                xMax=qMax(qMax(xMax,x1),qMax(x3,x2));
                yMax=qMax(qMax(yMax,y1),qMax(y3,y2));
                zMax=qMax(qMax(zMax,z1),qMax(z3,z2));
                xMin=qMin(qMin(xMin,x1),qMin(x3,x2));
                yMin=qMin(qMin(yMin,y1),qMin(y3,y2));
                zMin=qMin(qMin(zMin,z1),qMin(z3,z2));

                v1=addVertex(x1,y1,z1);
                v2=addVertex(x2,y2,z2);
                v3=addVertex(x3,y3,z3);

                e1=addHalfEdge(v1,v2);
                e2=addHalfEdge(v2,v3);
                e3=addHalfEdge(v3,v1);

                //inserting face to list
                face=addFace(e1,e2,e3);
                e1->addFace(face);
                e2->addFace(face);
                e3->addFace(face);

                triang_count++;
                emit progress(file->pos(),file->size(), "Loading and analizing STL");
            }
        }
        //if binary
        else{
            QDataStream in(file);
            in.setByteOrder(QDataStream::LittleEndian);
            in.setFloatingPointPrecision(QDataStream::SinglePrecision);
            qint32 ntriangles;
            quint16 control_bytes;
            file->seek(80);
            in >> ntriangles;
            this->faces.reserve(ntriangles);
            while (triang_count < ntriangles) {
                file->seek(84+triang_count*50+0+0);
                in >> nx;
                file->seek(84+triang_count*50+0+4);
                in >> ny;
                file->seek(84+triang_count*50+0+8);
                in >> nz;
                file->seek(84+triang_count*50+12+0);
                in >> x1;
                file->seek(84+triang_count*50+12+4);
                in >> y1;
                file->seek(84+triang_count*50+12+8);
                in >> z1;
                file->seek(84+triang_count*50+24+0);
                in >> x2;
                file->seek(84+triang_count*50+24+4);
                in >> y2;
                file->seek(84+triang_count*50+24+8);
                in >> z2;
                file->seek(84+triang_count*50+36+0);
                in >> x3;
                file->seek(84+triang_count*50+36+4);
                in >> y3;
                file->seek(84+triang_count*50+36+8);
                in >> z3;
                file->seek(84+triang_count*50+48);
                in >> control_bytes;
                if (in.status() != QDataStream::Ok ){
                    qDebug()<< (int) in.status();
                    break;
                }
                if(triang_count==0){
                    xMax=x1;
                    yMax=y1;
                    zMax=z1;
                    xMin=x1;
                    yMin=y1;
                    zMin=z1;
                }

                xMax=qMax(qMax(xMax,x1),qMax(x3,x2));
                yMax=qMax(qMax(yMax,y1),qMax(y3,y2));
                zMax=qMax(qMax(zMax,z1),qMax(z3,z2));
                xMin=qMin(qMin(xMin,x1),qMin(x3,x2));
                yMin=qMin(qMin(yMin,y1),qMin(y3,y2));
                zMin=qMin(qMin(zMin,z1),qMin(z3,z2));

                v1=addVertex(x1,y1,z1);
                v2=addVertex(x2,y2,z2);
                v3=addVertex(x3,y3,z3);

                e1=addHalfEdge(v1,v2);
                e2=addHalfEdge(v2,v3);
                e3=addHalfEdge(v3,v1);
                //inserting face to list
                face=addFace(e1,e2,e3);
                e1->addFace(face);
                e2->addFace(face);
                e3->addFace(face);

                triang_count++;
                emit progress(file->pos(),file->size(), "Loading and analizing STL");
            }
        }
        //set center and offset
        qDebug() << this->offset;
        QHash<QString, Vertex*>::iterator i;
        this->offset=Vertex((xMax+xMin)/2,(yMax+yMin)/2,zMin);

        for(i = this->vertexes.begin(); i != this->vertexes.end(); ++i){
            i.value()->setX((i.value()->x()-this->offset.x())*0.01);
            i.value()->setY((i.value()->y()-this->offset.y())*0.01);
            i.value()->setZ((i.value()->z()-this->offset.z())*0.01);
        }
        this->height=(zMax-zMin)*0.01;
        this->offset.setZ(0);
        this->offset = this->offset*0.01;
        qDebug() << "ilosc lini:" << this->edges.size();
        qDebug() << "ilosc verteksow:" << this->vertexes.size();
        qDebug() << "ilosc facy:" << this->faces.size();
        qDebug() << "ilosc zlych edgy: " << this->badEdges.size();

        //check all edges to find non manifold ones, and nad normals
        QMap<QString,HalfEdge*>::iterator j;
        for (j = this->edges.begin(); j != this->edges.end(); ++j){
            if(j.value()->getFaces().size()==0 && j.value()->getTwin()->getFaces().size()==1){
                this->badEdges.append(j.value()->getTwin()->getHash());
            }
            if(j.value()->getFaces().size()==2 && j.value()->getTwin()->getFaces().size()==0){
                badNormals=true;
            }
        }

        if(this->badEdges.size()>0){
            this->nonManifold=true;
        }
    }
    else{
        qDebug() << "Error" << file->errorString();
    }
    delete file;
    emit doneProcessing(true);

    //if there are bad normals then recaculate them
    if(badNormals){
        repairNormals();
    }
    qDebug() << "loading took: " <<myTimer.elapsed();
}

//adding verteces without adding doubles
Vertex* StlObject::addVertex(float x, float y, float z){
    Vertex* out;
    //merging close vetreces so stiching algorythm have less work
    float threshold=0.005;
    float roundedX=(int)(x/threshold)*threshold;
    float roundedY=(int)(y/threshold)*threshold;
    float roundedZ=(int)(z/threshold)*threshold;
    QString hash=QString::number(roundedX)+QString::number(roundedY)+QString::number(roundedZ);
    //adding verteces without adding doubles
    if(this->vertexes.contains(hash)){
       out=this->vertexes.value(hash);
    }
    else{
        out=new Vertex();
        out->setX(x);
        out->setY(y);
        out->setZ(z);
        this->vertexes.insert(hash,out);
    }
    return out;
}
//adding edges
HalfEdge* StlObject::addHalfEdge(Vertex *v1, Vertex *v2){
    HalfEdge* out;
    QString hash=QString::number((int)v1)+QString::number((int)v2);

    if(!this->edges.contains(hash)){
        out=new HalfEdge(v1,v2);
        this->edges.insert(hash,out);
        out->setTwin(addHalfEdge(v2,v1));
    }
    else{
        out=this->edges.value(hash);
    }

    return out;
}

//adding face
Face* StlObject::addFace(HalfEdge *edge1, HalfEdge *edge2, HalfEdge *edge3){
    QString hash;
    QList<int> edges;
    edges.append((int)edge1);
    edges.append((int)edge2);
    edges.append((int)edge3);
    //sort edges so all faces using these have same hash
    qSort(edges);
    hash.append(QString::number(edges.at(0))+QString::number(edges.at(1))+QString::number(edges.at(2)));
    Face* out;

    if(this->faces.contains(hash)){
        return this->faces.value(hash);
    }
    else{
        out = new Face(edge1,edge2,edge3);
        this->faces.insert(hash,out);
        return out;
    }
}

void StlObject::select(bool value){
    this->selected=value;
}

void StlObject::moveXY(double x, double y){
    this->offset.setX(x);
    this->offset.setY(y);
}

QVector3D StlObject::getOffset(){
    return this->offset;
}

void StlObject::scale(double value){
    if(value>0){
        this->scaleValue=value;
    }
}

double StlObject::getScale(){
    return this->scaleValue;
}

void StlObject::rotate(double angle){
    this->rotation=angle;
}

double StlObject::getRotation(){
    return this->rotation;
}

void StlObject::draw(bool picking){
    GLfloat   gray[4]={0.9,0.9,0.9,1.0};
    GLfloat   white[4]={0.9,1.0,0.8,1.0};
    GLfloat   red[4]={1.0,0.8,0.8,1.0};
    GLfloat   green[4]={0.8,1.0,0.8,1.0};
    GLfloat   reder[4]={1.0,0.0,0.0,1.0};
    GLfloat   fCurrentColor[4];
    GLfloat   lastColor[4];
    //iterator
    QHash<QString, Face*>::iterator i;
    // Get the current color
    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);
    if(!picking){
        if(selected){
            glColor4fv(white);
        }
        else if(colliding){
            glColor4fv(red);
        }
        else{
            glColor4fv(gray);
        }
    }
    glPushMatrix();
    glTranslatef(0.0+this->offset.x(), 0.0+this->offset.y(), 0.0);
    glRotatef(((int)this->rotation/5)*5, 0.0, 0.0, 1.0);
    glBegin(GL_TRIANGLES);
    for(i = this->faces.begin(); i != this->faces.end(); ++i){
        glNormal3f(i.value()->getNormal()->x(),i.value()->getNormal()->y(),i.value()->getNormal()->z());
        glVertex3f(i.value()->getEdge1()->getStart()->x()*this->scaleValue, i.value()->getEdge1()->getStart()->y()*this->scaleValue, i.value()->getEdge1()->getStart()->z()*this->scaleValue);
        glVertex3f(i.value()->getEdge2()->getStart()->x()*this->scaleValue, i.value()->getEdge2()->getStart()->y()*this->scaleValue, i.value()->getEdge2()->getStart()->z()*this->scaleValue);
        glVertex3f(i.value()->getEdge3()->getStart()->x()*this->scaleValue, i.value()->getEdge3()->getStart()->y()*this->scaleValue, i.value()->getEdge3()->getStart()->z()*this->scaleValue);
    }
    glEnd( );

    if(!picking){
        //draw bad edges
        glDisable(GL_LIGHTING);
        glColor4fv(reder);
        glLineWidth(2.0);
        glDepthMask(false);
        glBegin(GL_LINES);
        for(int i=0; i<this->badEdges.size(); i++){
            glVertex3f(this->edges.value(this->badEdges.at(i))->getStart()->x()*this->scaleValue, this->edges.value(this->badEdges.at(i))->getStart()->y()*this->scaleValue, this->edges.value(this->badEdges.at(i))->getStart()->z()*this->scaleValue);
            glVertex3f(this->edges.value(this->badEdges.at(i))->getStop()->x()*this->scaleValue, this->edges.value(this->badEdges.at(i))->getStop()->y()*this->scaleValue, this->edges.value(this->badEdges.at(i))->getStop()->z()*this->scaleValue);
        }
        glEnd( );
        glDepthMask(true);
        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
    // Restore the current color
    glColor4fv(fCurrentColor);
}




void StlObject::mirror(QChar axis){
    QMatrix4x4 matrix;
    Vertex offset;
    switch(axis.toAscii()){
    case 'x':
        offset = Vertex(0,0,0);
        matrix.scale(-1.0,1.0,1.0);
        break;
    case 'y':
        offset = Vertex(0,0,0);
        matrix.scale(1.0,-1.0,1.0);
        break;
    case 'z':
        offset = Vertex(0,0,this->height);
        matrix.scale(1.0,1.0,-1.0);
        break;
    }
    //zamien rotacje vertexow robiaz zamiane v2 i v3 powinno dzialac
    //transform verteces
    QHash<QString, Vertex*>::iterator i;
    for(i = this->vertexes.begin(); i != this->vertexes.end(); ++i){
        *i.value()=((*i.value()*matrix)+offset);
    }
    QHash<QString, Face*>::iterator j;
    for(j = this->faces.begin(); j != this->faces.end(); ++j){
       j.value()->flip();
    }
}

QList<QVector3D> StlObject::getTriangles(){
    QList<QVector3D> out;
    QMatrix4x4 matrix;
    matrix.rotate(360-((((int)this->rotation/5)*5)%360),0.0f,0.0f,1.0f);
    QHash<QString, Face*>::iterator j;
    for(j = this->faces.begin(); j != this->faces.end(); ++j){
        out.append((((*j.value()->getNormal())*matrix)));
        out.append((((*j.value()->getEdge1()->getStart())*matrix)*(100*this->scaleValue))+this->offset*100);
        out.append((((*j.value()->getEdge2()->getStart())*matrix)*(100*this->scaleValue))+this->offset*100);
        out.append((((*j.value()->getEdge3()->getStart())*matrix)*(100*this->scaleValue))+this->offset*100);
    }
    return out;
}

void StlObject::repairNormals(){
    QList<Face*> tempList;
    QList<Face*> facesToCheck;
    QList<Face*> unOrientedFaces;
    double minimalDist=500;
    Face *currentFace;
    Face *startFace;
    unOrientedFaces=this->faces.values();
    int maximum=unOrientedFaces.size();
    qDebug() << unOrientedFaces.size();
    QHash<QString, Face*>::iterator j;
    for(j = this->faces.begin(); j != this->faces.end(); ++j){
        j.value()->setNormalIsGood(false);
    }

    while(unOrientedFaces.size()>0){
        minimalDist=500;
        //finding closes face to some plane to find out if normal vector is pointing the right way
        for(int i=0; i<unOrientedFaces.size(); i++){
            if(unOrientedFaces.at(i)->getCenter().distanceToPlane(QVector3D(0,0,-100),QVector3D(0,0,1))<minimalDist){
                minimalDist=unOrientedFaces.at(i)->getCenter().distanceToPlane(QVector3D(0,0,-100),QVector3D(0,0,1));
                startFace=unOrientedFaces.at(i);
            }
        }
        //checking if normals are pointing outside
        if(((acos(QVector3D::dotProduct(startFace->getNormal()->normalized(),QVector3D(0,0,-100).normalized()))*(double)180)/3.14159)>91){
            startFace->flip();
        }
        startFace->setNormalIsGood(true);
        facesToCheck.append(startFace);
        //check all edges of face
        //check edge1
        //if its ok
        while(facesToCheck.size()>0){
            currentFace=facesToCheck.takeFirst();
            unOrientedFaces.removeAt(unOrientedFaces.indexOf(currentFace));
            if(currentFace->getEdge1()->getFaces().size()==1 && currentFace->getEdge1()->getTwin()->getFaces().size()==1){
                //did we check neighbor?
                if(!currentFace->getEdge1()->getTwin()->getFaces().at(0)->isNormalGood()){
                    currentFace->getEdge1()->getTwin()->getFaces().at(0)->setNormalIsGood(true);
                    //if not then set its ok and check it neighbors
                    facesToCheck.append(currentFace->getEdge1()->getTwin()->getFaces().at(0));
                }
            }
            //if half edge is used twice
            if(currentFace->getEdge1()->getFaces().size()==2 && currentFace->getEdge1()->getTwin()->getFaces().size()==0){
                //get faces od this half edge
                tempList = currentFace->getEdge1()->getFaces();
                //remove from list current edge
                tempList.removeAt(tempList.indexOf(currentFace));
                if(!tempList.at(0)->isNormalGood()){
                    tempList.at(0)->flip();
                    tempList.at(0)->setNormalIsGood(true);
                    facesToCheck.append(tempList.at(0));
                }
            }
            if(currentFace->getEdge2()->getFaces().size()==1 && currentFace->getEdge2()->getTwin()->getFaces().size()==1){
                if(!currentFace->getEdge2()->getTwin()->getFaces().at(0)->isNormalGood()){
                    currentFace->getEdge2()->getTwin()->getFaces().at(0)->setNormalIsGood(true);
                    facesToCheck.append(currentFace->getEdge2()->getTwin()->getFaces().at(0));
                }
            }
            if(currentFace->getEdge2()->getFaces().size()==2 && currentFace->getEdge2()->getTwin()->getFaces().size()==0){
                tempList = currentFace->getEdge2()->getFaces();
                tempList.removeAt(tempList.indexOf(currentFace));
                if(!tempList.at(0)->isNormalGood()){
                    tempList.at(0)->flip();
                    tempList.at(0)->setNormalIsGood(true);
                    facesToCheck.append(tempList.at(0));
                }
            }
            if(currentFace->getEdge3()->getFaces().size()==1 && currentFace->getEdge3()->getTwin()->getFaces().size()==1){
                if(!currentFace->getEdge3()->getTwin()->getFaces().at(0)->isNormalGood()){
                    currentFace->getEdge3()->getTwin()->getFaces().at(0)->setNormalIsGood(true);
                    facesToCheck.append(currentFace->getEdge3()->getTwin()->getFaces().at(0));
                }
            }
            if(currentFace->getEdge3()->getFaces().size()==2 && currentFace->getEdge3()->getTwin()->getFaces().size()==0){
                tempList = currentFace->getEdge3()->getFaces();
                tempList.removeAt(tempList.indexOf(currentFace));
                if(!tempList.at(0)->isNormalGood()){
                    tempList.at(0)->flip();
                    tempList.at(0)->setNormalIsGood(true);
                    facesToCheck.append(tempList.at(0));
                }
            }
            emit progress(maximum-unOrientedFaces.size(),maximum, tr("Calculating normal vectors"));
        }
    }
    emit doneProcessing(true);
}

void StlObject::repairHoles(){
    this->badEdges.clear();
    QMap<QString,HalfEdge*>::iterator i;
    for (i = this->edges.begin(); i != this->edges.end(); ++i){
        //if half edge is non manifold
        if(i.value()->getFaces().size()==0 && i.value()->getTwin()->getFaces().size()==1){
            this->badEdges.append(i.value()->getTwin()->getHash());
        }
    }
//

//    qDebug() << this->startFace->getEdge2()->getFaces().size();
//    qDebug() << this->startFace->getEdge3()->getFaces().size();
}
