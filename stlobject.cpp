#include "stlobject.h"

//stl object
StlObject::StlObject(QObject *parent) :
QObject(parent)
{
    this->slicer=new Slice();
    this->scaleValue=1.0;
    this->rotation=0.0;
    this->selected=false;
    this->colliding=false;
    this->nonManifold=false;
    this->mirrorX=false;
    this->mirrorY=false;
    this->mirrorZ=false;
}

StlObject::StlObject(QString fileName, QObject *parent) :
    QObject(parent)
{
    this->slicer=new Slice();
    this->scaleValue=1.0;
    this->rotation=0.0;
    this->selected=false;
    this->colliding=false;
    this->nonManifold=false;
    this->mirrorX=false;
    this->mirrorY=false;
    this->mirrorZ=false;
    this->loadFile(fileName);
}

void StlObject::copy(StlObject *copyFrom){
    qDebug() << copyFrom->fileName;
    this->scaleValue=copyFrom->scaleValue;
    this->rotation=copyFrom->rotation;
    this->selected=false;
    this->colliding=false;
    this->nonManifold=copyFrom->nonManifold;

    this->vertexes=copyFrom->vertexes;
    this->faces=copyFrom->faces;
    this->edges=copyFrom->edges;

    this->width=copyFrom->width;
    this->height=copyFrom->height;
    this->lenght=copyFrom->lenght;
    this->fileName=copyFrom->fileName;
    this->offset=copyFrom->offset;

    this->mirrorX=copyFrom->mirrorX;
    this->mirrorY=copyFrom->mirrorY;
    this->mirrorZ=copyFrom->mirrorZ;
    this->list_index=copyFrom->list_index;
    this->objectMaterial = copyFrom->objectMaterial;
    this->slicer = copyFrom->slicer;
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
            qDebug() << "ascii";
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
                line=file->readLine();
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

                triang_count++;
                emit progress(file->pos(),file->size(), tr("Loading and analizing STL"));
            }
        }
        //if binary
        else{
            qDebug() << "binary";
            QDataStream in(file);
            in.setByteOrder(QDataStream::LittleEndian);
            in.setFloatingPointPrecision(QDataStream::SinglePrecision);
            qint32 ntriangles;
            quint16 control_bytes;
            file->seek(80);
            in >> ntriangles;
            this->vertexes.reserve(ntriangles*2);
            this->edges.reserve(ntriangles*3);
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

                triang_count++;
                emit progress(file->pos(),file->size(), "Loading and analizing STL");
            }
        }
        //set center and offset
        qDebug() << this->offset;
        QHash<QString, Vertex*>::iterator i;
        this->offset=QVector3D((xMax+xMin)/2,(yMax+yMin)/2,zMin);

        for(i = this->vertexes.begin(); i != this->vertexes.end(); ++i){
            i.value()->setX((i.value()->x()-this->offset.x()));
            i.value()->setY((i.value()->y()-this->offset.y()));
            i.value()->setZ((i.value()->z()-this->offset.z()));
        }
        this->height=(zMax-zMin)*0.01;
        this->width=(xMax-xMin)*0.01;
        this->lenght=(yMax-yMin)*0.01;
        this->offset.setZ(0);
        this->offset = this->offset*0.01;

        //check all edges to find non manifold ones, and nad normals
        QHash<QString,HalfEdge*>::iterator j;
        for (j = this->edges.begin(); j != this->edges.end(); ++j){
            if(j.value()->getFaces().size()==0 && j.value()->getTwin()->getFaces().size()==1){
                this->badEdges.append(j.value()->getTwin());
            }
            if(j.value()->getFaces().size()==2 && j.value()->getTwin()->getFaces().size()==0){
                badNormals=true;
            }
        }
        qDebug() << "ilosc lini:" << this->edges.size();
        qDebug() << "ilosc verteksow:" << this->vertexes.size();
        qDebug() << "ilosc facy:" << this->faces.size();
        qDebug() << "ilosc zlych edgy: " << this->badEdges.size();
        render();
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
    slicer->setLayerHeight(0.5);
    slicer->fillTriLayer(this->faces);
    qDebug() << "loading took: " <<myTimer.elapsed();
}


void StlObject::render(){
    GLuint index = glGenLists(1);
    this->list_index=index;
    glFinish();
    //iterator
    QHash<QString, Face*>::iterator i;
    glNewList(this->list_index, GL_COMPILE);
    for(i = this->faces.begin(); i != this->faces.end(); ++i){
        glBegin(GL_TRIANGLES);
        glNormal3f(i.value()->getNormal()->x(),i.value()->getNormal()->y(),i.value()->getNormal()->z());
        glVertex3f(i.value()->getEdge1()->getStart()->x(), i.value()->getEdge1()->getStart()->y(), i.value()->getEdge1()->getStart()->z());
        glVertex3f(i.value()->getEdge2()->getStart()->x(), i.value()->getEdge2()->getStart()->y(), i.value()->getEdge2()->getStart()->z());
        glVertex3f(i.value()->getEdge3()->getStart()->x(), i.value()->getEdge3()->getStart()->y(), i.value()->getEdge3()->getStart()->z());
        glEnd( );
    }
    glEndList();
}

//adding verteces without adding doubles
Vertex* StlObject::addVertex(float x, float y, float z){
    Vertex* out;
    //merging close vetreces so stiching algorythm have less work
    float threshold=0.002;
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
    QString hash=QString::number((size_t)v1)+QString::number((size_t)v2);

    if(!this->edges.contains(hash)){
        out=new HalfEdge(v1,v2);
        this->edges.insert(hash,out);
        out->setTwin(addHalfEdge(v2,v1));
    }
    else{
        out=this->edges.value(hash);
    }
    //add edges to verteces
    v1->addHalEdge(out);
    v2->addHalEdge(out);
    return out;
}

//adding face
Face* StlObject::addFace(HalfEdge *edge1, HalfEdge *edge2, HalfEdge *edge3){
    QString hash;
    QList<size_t> edges;
    edges.append((size_t)edge1);
    edges.append((size_t)edge2);
    edges.append((size_t)edge3);
    //sort edges so all faces using these have same hash
    qSort(edges);
    hash.append(QString::number(edges.at(0))+QString::number(edges.at(1))+QString::number(edges.at(2)));
    Face* out;

    if(this->faces.contains(hash)){
        out = this->faces.value(hash);
    }
    else{
        out = new Face(edge1,edge2,edge3);
        this->faces.insert(hash,out);
    }
    edge1->addFace(out);
    edge2->addFace(out);
    edge3->addFace(out);
    return out;
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
    if(this->mirrorX^this->mirrorY){
        this->rotation-=angle;
    }
    else{
        this->rotation+=angle;
    }

    if(abs(this->rotation)>360){
        this->rotation-=360*((int)this->rotation/360);
    }
}

void StlObject::setRotation(double angle){
    this->rotation=angle;
}

double StlObject::getRotation(){
    return ((int)this->rotation/5)*5;
}

void StlObject::draw(bool picking, bool showLayers, int layerNum){
    GLfloat   reder[4]={1.0,0.0,0.0,1.0};
    GLfloat     ambient[] = { 0.1,   0.1,     0.1,  1.0};
    GLfloat     diffuse[] = {this->objectMaterial->getColor().redF(),  this->objectMaterial->getColor().greenF(),    this->objectMaterial->getColor().blueF(), 1.0};
    GLfloat     specular[] = {0.50,  0.50,    0.50, 1.0};
    GLfloat     shininess[] = {200};
    GLfloat     ambient_S[] = { 0.1,   0.1,     0.1,  1.0};
    GLfloat     diffuse_S[] = {this->objectMaterial->getColor().lighter(175).redF(),   this->objectMaterial->getColor().lighter(125).greenF(),this->objectMaterial->getColor().lighter(125).blueF(),1.0};
    GLfloat     specular_S[] = {0.50,  0.50,    0.50, 1.0};
    GLfloat     shininess_S[] = {200};
    GLfloat   fCurrentColor[4];
    // Get the current color
    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);
        glDisable(GL_COLOR_MATERIAL);
    if(!picking){
        if(selected){
            glMaterialfv(GL_FRONT, GL_AMBIENT,ambient_S);
            glMaterialfv(GL_FRONT, GL_DIFFUSE,diffuse_S);
            glMaterialfv(GL_FRONT, GL_SPECULAR,specular_S);
            glMaterialfv(GL_FRONT, GL_SHININESS,shininess_S);
        }
        else{
            glMaterialfv(GL_FRONT, GL_AMBIENT,ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE,diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR,specular);
            glMaterialfv(GL_FRONT, GL_SHININESS,shininess);
        }
    }
    glPushMatrix();
    glTranslatef(0.0+this->offset.x(), 0.0+this->offset.y(), (this->mirrorZ ? getHeight(): 0.0));

    glScalef( (this->mirrorX ? -1.0 : 1.0)*this->scaleValue*0.01, (this->mirrorY ? -1.0 : 1.0)*this->scaleValue*0.01, (this->mirrorZ ? -1.0 : 1.0)*this->scaleValue*0.01);

    glRotatef(((int)this->rotation/5)*5, 0.0, 0.0, 1.0);
    //call list
    if(picking || !showLayers){
        glCallList(this->list_index);
    }
    else{
        this->slicer->draw(layerNum);
    }
    if(!picking){
        //draw bad edges
        glDisable(GL_LIGHTING);
        glColor4fv(reder);
        glLineWidth(2.0);
        glDepthMask(false);
        glBegin(GL_LINES);
        for(int i=0; i<this->badEdges.size(); i++){
            glVertex3f(this->badEdges.at(i)->getStart()->x(), this->badEdges.at(i)->getStart()->y(), this->badEdges.at(i)->getStart()->z());
            glVertex3f(this->badEdges.at(i)->getStop()->x(), this->badEdges.at(i)->getStop()->y(), this->badEdges.at(i)->getStop()->z());
        }
        glEnd( );
        glDepthMask(true);
        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
    // Restore the current color
    glEnable(GL_COLOR_MATERIAL);
    glColor4fv(fCurrentColor);
}




void StlObject::mirror(QChar axis){
    switch(axis.toAscii()){
    case 'x':
        this->mirrorX=!this->mirrorX;
        break;
    case 'y':
        this->mirrorY=!this->mirrorY;
        break;
    case 'z':
        this->mirrorZ=!this->mirrorZ;
        break;
    }
}

QList<QVector3D> StlObject::getTriangles(){
    QList<QVector3D> out;
    QMatrix4x4 matrix;
    QVector3D zOffset(0,0,(this->mirrorZ ? getHeight()*100 : 0.0));

    matrix.rotate(360-((((int)this->rotation/5)*5)%360),0.0f,0.0f,1.0f);
    matrix.scale((this->mirrorX ? -1.0 : 1.0), (this->mirrorY ? -1.0 : 1.0), (this->mirrorZ ? -1.0 : 1.0));
    QHash<QString, Face*>::iterator j;
    for(j = this->faces.begin(); j != this->faces.end(); ++j){
        out.append((((*j.value()->getNormal())*matrix)));
        //if mirror change vertex winding order
        if(this->mirrorX ^ this->mirrorY ^ this->mirrorZ){
            out.append((((*j.value()->getEdge3()->getStart())*matrix)*(this->scaleValue))+this->offset*100+zOffset);
            out.append((((*j.value()->getEdge2()->getStart())*matrix)*(this->scaleValue))+this->offset*100+zOffset);
            out.append((((*j.value()->getEdge1()->getStart())*matrix)*(this->scaleValue))+this->offset*100+zOffset);
        }
        else{
            out.append((((*j.value()->getEdge1()->getStart())*matrix)*(this->scaleValue))+this->offset*100+zOffset);
            out.append((((*j.value()->getEdge2()->getStart())*matrix)*(this->scaleValue))+this->offset*100+zOffset);
            out.append((((*j.value()->getEdge3()->getStart())*matrix)*(this->scaleValue))+this->offset*100+zOffset);
        }
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
    //rerender mesh
    glDeleteLists(this->list_index, 1);
    render();
    emit doneProcessing(true);
}

void StlObject::repairHoles(){
    this->badEdges.clear();
    QHash<QString,HalfEdge*>::iterator i;
    //first find non manifold edges
    for (i = this->edges.begin(); i != this->edges.end(); ++i){
        //if half edge is non manifold
        if(i.value()->getFaces().size()==0 && i.value()->getTwin()->getFaces().size()==1){
            this->badEdges.append(i.value());
        }
    }
    //then check angles of each vertex in holes
    QVector3D p1,p2,p3;
    QList<HalfEdge*> tempList;
    QList< QPair<double,PatchFace> > edgesList;
    for(int j=0; j<this->badEdges.size(); j++){
        tempList=this->badEdges.at(j)->getStop()->getEdges();
        tempList=intersectLists(tempList,this->badEdges);
        tempList.removeAt(tempList.indexOf(this->badEdges.at(j)));
        p2=this->badEdges.at(j)->getStop()->toVector3D();
        PatchFace newFace;

        newFace.middlePoint=this->badEdges.at(j)->getStop();
        newFace.edge1=this->badEdges.at(j);
        newFace.edge2=tempList.at(0);

        //add edge to list
        newFace.angle=angle(newFace.edge1->getStart()->toVector3D()-p2,newFace.edge2->getStop()->toVector3D()-p2);
        edgesList.append(qMakePair(newFace.angle,newFace));
    }
    //after we have all edges and angles sort list
    qSort(edgesList.begin(), edgesList.end(), QPairFirstComparer());
    for(int j=0; j<edgesList.size(); j++){
        qDebug() << edgesList.at(j).first;
    }
    //now the fun part
    if(edgesList.size()>0){
        //there are three posibilitys
        PatchFace currentFace=edgesList.at(0).second;
        //angle is less than 75*
        HalfEdge *newEdge;
        Face *newFace;
        if(currentFace.angle<75){
            qDebug() << this->edges.size();
            qDebug() << "dodaje face";
            newEdge = addHalfEdge(currentFace.edge2->getStop(),currentFace.edge1->getStart());
            newFace=addFace(currentFace.edge1,currentFace.edge2,newEdge);
            qDebug() << newFace->getEdge1()->getStart() << newFace->getEdge2()->getStart() << newFace->getEdge3()->getStart();
            qDebug() << this->edges.size();
        }
    }
    //angle is less than 135*
    //angle is greater than 135*
}

QList<HalfEdge*> StlObject::intersectLists(QList<HalfEdge *> list1, QList<HalfEdge *> list2){
    QList<HalfEdge*> out;
    for(int i=0; i<list1.size(); i++){
        if(list2.contains(list1.at(i))){
            out.append(list1.at(i));
        }
    }
    return out;
}

double StlObject::angle(QVector3D p1, QVector3D p2){
    qDebug() << p1 << p2;
    return (acos(QVector3D::dotProduct(p1.normalized(),p2.normalized()))*(double)180)/3.14159;
}

QString StlObject::getFileName(){
    return this->fileName;
}

void StlObject::freeList(){
    glDeleteLists(this->list_index, 1);
}
