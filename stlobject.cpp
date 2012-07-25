#include "stlobject.h"

StlObject::StlObject(QString fileName,int name, QObject *parent) :
    QObject(parent)
{
    this->scaleValue=1.0;
    this->rotation=0.0;
    this->name=name;
    this->selected=false;
    this->colliding=false;
    this->fileName=fileName;
    QFile* file = new QFile(fileName);
    if(file->open(QIODevice::ReadOnly)){
        float xMax=0,xMin=0;
        float yMax=0,yMin=0;
        float zMax=0,zMin=0;
        int triang_count = 0;
        float nx, ny, nz, x1, x2, x3, y1, y2, y3, z1, z2, z3;
        QString line;
        //if ascii file
        if(file->readLine().contains("solid")){
            while(!file->atEnd()){
                line=file->readLine();
                if(line.contains("endsolid")){
                    break;
                }
                nx=line.split(" ", QString::SkipEmptyParts).at(2).toFloat();
                ny=line.split(" ", QString::SkipEmptyParts).at(3).toFloat();
                nz=line.split(" ", QString::SkipEmptyParts).at(4).toFloat();
                line=file->readLine();
                line=file->readLine();
                x1=line.split(" ",QString::SkipEmptyParts).at(1).toFloat();
                y1=line.split(" ",QString::SkipEmptyParts).at(2).toFloat();
                z1=line.split(" ",QString::SkipEmptyParts).at(3).toFloat();
                line=file->readLine();
                x2=line.split(" ",QString::SkipEmptyParts).at(1).toFloat();
                y2=line.split(" ",QString::SkipEmptyParts).at(2).toFloat();
                z2=line.split(" ",QString::SkipEmptyParts).at(3).toFloat();
                line=file->readLine();
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

                this->vertexes.append(ComputeFaceNormal(QVector3D(x1,y1,z1),QVector3D(x2,y2,z2),QVector3D(x3,y3,z3)));
                this->vertexes.append(QVector3D(x1,y1,z1));
                this->vertexes.append(QVector3D(x2,y2,z2));
                this->vertexes.append(QVector3D(x3,y3,z3));
                triang_count++;
            }
        }
        //if binary
        else{
            QDataStream in(file);
            in.setByteOrder(QDataStream::LittleEndian);
            in.setFloatingPointPrecision(QDataStream::SinglePrecision);
            quint32 ntriangles;
            quint16 control_bytes;
            file->seek(80);
            in >> ntriangles;
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

                this->vertexes.append(ComputeFaceNormal(QVector3D(x1,y1,z1),QVector3D(x2,y2,z2),QVector3D(x3,y3,z3)));
                this->vertexes.append(QVector3D(x1,y1,z1));
                this->vertexes.append(QVector3D(x2,y2,z2));
                this->vertexes.append(QVector3D(x3,y3,z3));
                triang_count++;
            }
        }

        //set center and offset
        this->offset=QVector3D((xMax+xMin)/2,(yMax+yMin)/2,zMin);
        qDebug() << this->offset;
        for(int i=0; i<vertexes.size(); i+=4){
            vertexes.replace(i+1,(vertexes.at(i+1)-this->offset)*0.01);
            vertexes.replace(i+2,(vertexes.at(i+2)-this->offset)*0.01);
            vertexes.replace(i+3,(vertexes.at(i+3)-this->offset)*0.01);
        }
        this->height=(zMax-zMin)*0.01;

        this->offset.setZ(0);
        this->offset = this->offset*0.01;
    }
    else{
        qDebug() << "Error" << file->errorString();
    }
    delete file;
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
    GLfloat fCurrentColor[4];
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
    for(int i=0; i<vertexes.size(); i+=4){
        glNormal3f(vertexes.at(i).x(), vertexes.at(i).y(), vertexes.at(i).z());
        glVertex3f(vertexes.at(i+1).x()*this->scaleValue, vertexes.at(i+1).y()*this->scaleValue, vertexes.at(i+1).z()*this->scaleValue);
        glVertex3f(vertexes.at(i+2).x()*this->scaleValue, vertexes.at(i+2).y()*this->scaleValue, vertexes.at(i+2).z()*this->scaleValue);
        glVertex3f(vertexes.at(i+3).x()*this->scaleValue, vertexes.at(i+3).y()*this->scaleValue, vertexes.at(i+3).z()*this->scaleValue);
    }
    glEnd( );
    glPopMatrix();
    // Restore the current color
    glColor4fv(fCurrentColor);
}


// Offset pIn by pOffset into pOut
QVector3D StlObject::VectorOffset(QVector3D pIn, QVector3D pOffset)
{
    QVector3D out;
    out.setX(pIn.x() - pOffset.x());
    out.setY(pIn.y() - pOffset.y());
    out.setZ(pIn.z() - pOffset.z());
    return out;
}
// Compute the cross product a X b into pOut
QVector3D StlObject::VectorGetNormal(QVector3D a, QVector3D b)
{
    QVector3D out;
    out.setX(a.y() * b.z() - a.z() * b.y());
    out.setY(a.z() * b.x() - a.x() * b.z());
    out.setZ(a.x() * b.y() - a.y() * b.x());
    return out;
}

// Compute p1,p2,p3 face normal into pOut
QVector3D StlObject::ComputeFaceNormal(QVector3D p1, QVector3D p2, QVector3D p3)
{
    // Uses p2 as a new origin for p1,p3
    QVector3D a=VectorOffset(p3,p2);
    QVector3D b=VectorOffset(p1,p2);
    // Compute the cross product a X b to get the face normal
    QVector3D pn=VectorGetNormal(a, b);
    // Return a normalized vector
    return pn.normalized();
}


QList<QVector3D> StlObject::transform(){
    QList<QVector3D> out;
    QMatrix4x4 matrix;
    matrix.rotate(360-((((int)this->rotation/5)*5)%360),0.0f,0.0f,1.0f);
   // matrix.translate(this->offset);
    for(int i=0; i<this->vertexes.size(); i++){
        out.append((((this->vertexes.at(i))*matrix)*(100*this->scaleValue))+this->offset*100);
    }

    return out;
}

void StlObject::mirror(QChar axis){
    QMatrix4x4 matrix;
    QList<QVector3D> temp;
    QVector3D offset;
    switch(axis.toAscii()){
    case 'x':
        offset = QVector3D(0,0,0);
        matrix.scale(-1.0,1.0,1.0);
        break;
    case 'y':
        offset = QVector3D(0,0,0);
        matrix.scale(1.0,-1.0,1.0);
        break;
    case 'z':
        offset = QVector3D(0,0,this->height);
        matrix.scale(1.0,1.0,-1.0);
        break;
    }
    for(int i=0; i<this->vertexes.size(); i+=4){
        temp.append((this->vertexes.at(i)*matrix));
        temp.append((this->vertexes.at(i+1)*matrix)+offset);
        temp.append((this->vertexes.at(i+2)*matrix)+offset);
        temp.append((this->vertexes.at(i+3)*matrix)+offset);
    }
    this->vertexes.clear();
    this->vertexes=temp;
}

QList<QVector3D> StlObject::getTriangles(){
    return this->vertexes;
}
