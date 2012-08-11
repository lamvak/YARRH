#include "layer.h"

Layer::Layer(int layer, QObject *parent) :
    QObject(parent)
{
    this->layer_num=layer;
}


QVector3D Layer::calculateNormal(QVector3D v1, QVector3D v2, QVector3D v3){
    QVector3D normal;
    qreal Qx, Qy, Qz, Px, Py, Pz;

       Qx = v2.x()-v1.x();
       Qy = v2.y()-v1.y();
       Qz = v2.z()-v1.z();
       Px = v3.x()-v1.x();
       Py = v3.y()-v1.y();
       Pz = v3.z()-v1.z();

       normal.setX(Py*Qz - Pz*Qy);
       normal.setY(Pz*Qx - Px*Qz);
       normal.setZ(Px*Qy - Py*Qx);
       normal.normalize();
    return normal;
}

void Layer::renderLine(QVector3D v1, QVector3D v2, qreal radius, qreal travel, bool last)
{
    qreal dx = v2.x() - v1.x();
    qreal dy = v2.y() - v1.y();
    qreal perp_x = dy;
    qreal perp_y= -dx;
    qreal len = sqrt( perp_x*perp_x + perp_y*perp_y );
    perp_x /= len;
    perp_y /= len;
    perp_x *= radius;
    perp_y *= radius;
    QVector3D normal_left= calculateNormal(v1,v2,QVector3D(v1.x(),v1.y(),v1.z()+radius));
    // Get the current color
        glBegin(GL_TRIANGLE_STRIP);
                glNormal3f( 0.0f, 0.0, -1.0f);
                glVertex3f(v1.x(), v1.y(), v1.z());        // V0
                glVertex3f(v2.x(), v2.y(), v1.z());  // V1
                glVertex3f( v1.x()+perp_x,  v1.y()+perp_y, v1.z());  // V2
                glVertex3f( v2.x()+perp_x, v2.y()+perp_y, v1.z());

                glNormal3f( -normal_left.x(), -normal_left.y(), normal_left.z());
                glVertex3f( v1.x()+perp_x,  v1.y()+perp_y, v1.z()+radius);  // V2
                glVertex3f( v2.x()+perp_x, v2.y()+perp_y, v1.z()+radius);

                glNormal3f( 0.0f, 0.0, 1.0f);
                glVertex3f(v1.x(),  v1.y(), v1.z()+radius);        // V0
                glVertex3f(v2.x(), v2.y(), v1.z()+radius);  // V1
                glNormal3f( normal_left.x(), normal_left.y(), normal_left.z());
                glVertex3f(v1.x(),  v1.y(), v1.z());        // V0
                glVertex3f(v2.x(), v2.y(), v1.z());  // V1
        glEnd();

}

void Layer::display(float scale, bool last, bool show_travel, int current_layer){
    GLfloat   gray[4]={0.9,0.9,0.9,1.0};
    GLfloat   red[4]={1.0,0.0,0.0,1.0};
    GLfloat   green[4]={0.0,1.0,0.0,1.0};
    GLfloat   blue[4]={0.0,0.0,1.0,1.0};
    GLfloat   orange[4]={1.0,0.5,0.0,1.0};
    GLfloat fCurrentColor[4];

    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);
    if(last)
       glColor4fv(red);
    else{
       glColor4fv(gray);
       //current and printer layer coloring for travel moves
       if(current_layer>this->layer_num)
           glColor4fv(green);
       else{
           if(current_layer==this->layer_num)
               glColor4fv(orange);
       }
    }
    glCallList(this->list_index);
    if(show_travel){
        glColor4fv(blue);
        glCallList(this->list_index+1);
    }
    glColor4fv(fCurrentColor);
}


void Layer::render(float scale){
    //generate model list
    GLuint index = glGenLists(2);
    this->list_index=index;
    qDebug() << this->list_index;
    // compile the display list, store a triangle in it
    glNewList(index, GL_COMPILE);

    for(int j=0; j<vertexes.size()-1; j++){
        if(vertexes.at(j+1).w()==1)
            continue;
        renderLine(vertexes.at(j).toVector3D()*scale,vertexes.at(j+1).toVector3D()*scale,0.3*scale,vertexes.at(j+1).w(),false);
    }
    glEndList();
    //generate travel list
    // compile the display list, store a triangle in it
    glNewList(index+1, GL_COMPILE);

    for(int j=0; j<vertexes.size()-1; j++){
        if(vertexes.at(j+1).w()==0)
            continue;
        renderLine(vertexes.at(j).toVector3D()*scale,vertexes.at(j+1).toVector3D()*scale,0.3*scale,vertexes.at(j+1).w(),false);
    }
    glEndList();
    this->vertexes.clear();
}

void Layer::freeLists(){
    glDeleteLists(this->list_index, 2);
}
