#include <QtGui>
#include <QtOpenGL>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>
#include "stlview.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

StlView::StlView(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    zoom = 2.5;
    xMove = 0;
    yMove = 0;
    clickedX=0.0;
    clickedY=0.0;
    theta=90;
    phi=90;
    r=20;
    eyeX=0;
    eyeY=0;
    eyeZ=r;
    upX=0;
    upY=1;
    upZ=0;

    sizeX=20;
    sizeY=20;

    qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
    objectSelected = false;
    this->setFocusPolicy(Qt::StrongFocus);
    //setting up collison detection
//    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
//    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
//    btVector3   worldAabbMin(-20000,-20000,-20000);
//    btVector3   worldAabbMax(20000,20000,20000);

//    btAxisSweep3*   broadphase = new btAxisSweep3(worldAabbMin,worldAabbMax);

//    collisionWorld = new btCollisionWorld(dispatcher,broadphase,collisionConfiguration);
}

//size hints
QSize StlView::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize StlView::sizeHint() const
{
    return QSize(800, 800);
}


//mouse press event
void StlView::mousePressEvent(QMouseEvent *event)
{
    GLint viewport[4];
    lastPos = event->pos();
    lastPosWorld = screenToWorld(lastPos.x(),lastPos.y());
    glGetIntegerv(GL_VIEWPORT,viewport);
    paintPicking();
    QColor pickedColor=QColor(this->grabFrameBuffer().pixel(lastPos)).toRgb();
    qDebug() << pickedColor.name();
    //if control is pressed
    if(event->modifiers()==Qt::ControlModifier){
        //if object is already selected then deselect it
        if(this->selectedObjects.contains(pickedColor.name())){
            this->objects.value(pickedColor.name())->select(false);
            this->selectedObjects.removeAt(this->selectedObjects.indexOf(pickedColor.name()));
        }
        //else select it
        else{
            if(pickedColor.name()!="#4e4e7f"){
                this->objects.value(pickedColor.name())->select(true);
                this->selectedObjects.append(pickedColor.name());
            }
        }
    }
    //if we are in single object selection mode
    else{
        //clear all selections
        if(this->selectedObjects.size()>0){
            for(int i=0; i<this->selectedObjects.size(); i++){
                this->objects.value(this->selectedObjects.at(i))->select(false);
            }
            this->selectedObjects.clear();
        }
        //if object is clicked
        if(this->objects.keys().contains(pickedColor.name())){
            //select it
            this->selectedObjects.append(pickedColor.name());
            this->objects.value(pickedColor.name())->select(true);
            lastObjectPos=this->objects.value(pickedColor.name())->getOffset().toPointF();
        }
    }
    updateGL();
    if(this->selectedObjects.size()>0){
        this->objectSelected=true;
    }
    else{
        this->objectSelected=false;
    }
    emit objectPicked(this->objectSelected);
    if(this->objectSelected){
        emit selectedCol(this->selectedObjects.last());
        emit selectedRotation(((((int)this->objects.value(this->selectedObjects.last())->getRotation()/5)*5)%360)*-1);
        emit selectedScale((int)(this->objects.value(this->selectedObjects.last())->getScale()*100));
        emit selectedCors(this->objects.value(this->selectedObjects.last())->getOffset().toPointF());
    }
}

void StlView::selectObject(QString name){
    //deselect old objects
    qDebug() << name;
    if(this->selectedObjects.size()>0){
        for(int i=0; i<this->selectedObjects.size(); i++){
            this->objects.value(this->selectedObjects.at(i))->select(false);
        }
        this->selectedObjects.clear();
    }

    this->objectSelected=true;
    this->selectedObjects.append(name);
    this->objects.value(this->selectedObjects.last())->select(true);
    //update ui
    emit selectedRotation(((((int)this->objects.value(this->selectedObjects.last())->getRotation()/5)*5)%360)*-1);
    emit selectedScale((int)(this->objects.value(this->selectedObjects.last())->getScale()*100));
    emit selectedCors(this->objects.value(this->selectedObjects.last())->getOffset().toPointF());
    updateGL();
}

//mouse release event
void StlView::mouseReleaseEvent(QMouseEvent *){

}

//mouse move event
void StlView::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    if(event->x()>0 && event->y()>0 && event->x()<this->width() && event->y()< this->height()){
        QPointF clicked=screenToWorld(lastPos.x(),lastPos.y());
        QPointF currentClicked=screenToWorld(event->x(),event->y());
        QLineF start(lastObjectPos.x(),lastObjectPos.y(),clicked.x(),clicked.y());
        QLineF current(lastObjectPos.x(),lastObjectPos.y(),currentClicked.x(),currentClicked.y());
        if(this->objectSelected)
        {
            if(event->buttons() & Qt::LeftButton){
                for(int i=0; i<this->selectedObjects.size(); i++){
                    this->objects.value(this->selectedObjects.at(i))->moveXY(this->objects.value(this->selectedObjects.at(i))->getOffset().x()+currentClicked.x()-clicked.x(),this->objects.value(this->selectedObjects.at(i))->getOffset().y()+currentClicked.y()-clicked.y());
                }
               //this->objects.value(this->object)->moveXY(clicked.x()-lastPosWorld.x(),clicked.y()-lastPosWorld.y());
                emit selectedCors(this->objects.value(this->selectedObjects.last())->getOffset().toPointF());
            }
            if(event->buttons() & Qt::RightButton){
                this->objects.value(this->selectedObjects.last())->scale(this->objects.value(this->selectedObjects.last())->getScale()+(double)dx*0.01);
                emit selectedScale((int)(this->objects.value(this->selectedObjects.last())->getScale()*100));
            }
            if(event->buttons() & Qt::MiddleButton){
                this->objects.value(this->selectedObjects.last())->rotate(this->objects.value(this->selectedObjects.last())->getRotation()-start.angleTo(current));
                emit selectedRotation(((((int)this->objects.value(this->selectedObjects.last())->getRotation()/5)*5)%360)*-1);
            }
            updateGL();
        }
        else{
            if (event->buttons() & Qt::MiddleButton) {

            } else if (event->buttons() & Qt::LeftButton) {

                // Mouse point to angle conversion
                theta -= dy*1.0;    //3.0 rotations possible
                phi -= dx*1.0;

                // Spherical to Cartesian conversion.
                // Degrees to radians conversion factor 0.0174532
                eyeX = r * cos(theta*0.0174532) * cos(phi*0.0174532);
                eyeY = r * cos(theta*0.0174532) * sin(phi*0.0174532);
                eyeZ = r * sin(theta*0.0174532);

                // Reduce theta slightly to obtain another point on the same longitude line on the sphere.
                GLfloat dt=0.01;
                GLfloat eyeXtemp = r * cos(theta*0.0174532-dt) * cos(phi*0.0174532);
                GLfloat eyeYtemp = r * cos(theta*0.0174532-dt) * sin(phi*0.0174532);
                GLfloat eyeZtemp = r * sin(theta*0.0174532-dt);

                // Connect these two points to obtain the camera's up vector.
                upX=eyeXtemp-eyeX;
                upY=eyeYtemp-eyeY;
                upZ=eyeZtemp-eyeZ;
                updateGL();

            } else if (event->buttons() & Qt::RightButton){
                xMove -= clicked.x()-currentClicked.x();
                yMove -= clicked.y()-currentClicked.y();
                updateGL();
            }
        }

        lastPos = event->pos();
    }
}
//wheel event
void StlView::wheelEvent(QWheelEvent * event){

    if((r-(float)event->delta()/100)>0){
        r -= (float)event->delta()/100;
        eyeX = r * cos(theta*0.0174532) * cos(phi*0.0174532);
        eyeY = r * cos(theta*0.0174532) * sin(phi*0.0174532);
        eyeZ = r * sin(theta*0.0174532);

     // Reduce theta slightly to obtain another point on the same longitude line on the sphere.
        GLfloat dt=0.01;
        GLfloat eyeXtemp = r * cos(theta*0.0174532-dt) * cos(phi*0.0174532);
        GLfloat eyeYtemp = r * cos(theta*0.0174532-dt) * sin(phi*0.0174532);
        GLfloat eyeZtemp = r * sin(theta*0.0174532-dt);

     // Connect these two points to obtain the camera's up vector.
        upX=eyeXtemp-eyeX;
        upY=eyeYtemp-eyeY;
        upZ=eyeZtemp-eyeZ;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective( 10.0, (float)this->width()/(float)this->height(), 1.0, 200.0 );
        glMatrixMode(GL_MODELVIEW);
        updateGL();
    }
}

//inizalization of gl

void StlView::initializeGL()
{
    qglClearColor(QColor(78,78,127));
//    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
 //   glEnable(GL_CULL_FACE); //culling off so that bad normals are displayed
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}
//resize event
void StlView::resizeGL(int width, int height)
{
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 10.0, (float)width/(float)height, 1.0, 200.0 );
    glMatrixMode(GL_MODELVIEW);
}


void StlView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, -xMove, -yMove, 0.0, upX, upY, upZ);
    drawAxis();
    drawGrid();

    for(int i=0; i<this->objects.size(); i++){
        this->objects.values().at(i)->draw(false);
    }
}

void StlView::paintPicking(){
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    GLfloat fCurrentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);

    gluLookAt(eyeX, eyeY, eyeZ, -xMove, -yMove, 0.0, upX, upY, upZ);
    for(int i=0; i<this->objects.size(); i++){
        glColor3ub(QColor(this->objects.keys().at(i)).red(),QColor(this->objects.keys().at(i)).green(),QColor(this->objects.keys().at(i)).blue());
        this->objects.values().at(i)->draw(true);
    }
    glFlush();
    glColor4fv(fCurrentColor);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DITHER);
}

QPointF StlView::screenToWorld(int x, int y){
   // get line of sight through mouse cursor
    GLint viewport[4];
     GLdouble modelview[16];
     GLdouble projection[16];
     GLfloat winX, winY, winZ;
     GLdouble posX, posY, posZ;

     glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
     glGetDoublev( GL_PROJECTION_MATRIX, projection );
     glGetIntegerv( GL_VIEWPORT, viewport );
     glClear(GL_DEPTH_BUFFER_BIT);

     GLfloat fCurrentColor[4];
     glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);
     glColor4f(0.5f, 0.5f, 0.5f, 0.0f);
     glBegin(GL_QUADS);
     glVertex3f(-20.0f,-20.0f, 0.00f);
     glVertex3f( 20.0f,-20.0f, 0.00f);
     glVertex3f( 20.0f, 20.0f, 0.00f);
     glVertex3f(-20.0f, 20.0f, 0.00f);
     glEnd();
     glColor4fv(fCurrentColor);


     winX = (float)x;
     winY = (float)viewport[3] - (float)y;
     glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
     gluUnProject( winX, winY,  winZ, modelview, projection, viewport, &posX, &posY, &posZ);

   return QPointF(posX,posY);
}

//drawing 20x20cm grind in 3dview
void StlView::drawGrid(){
    GLint     iSlices=32;
    GLint     iStacks=32;
    GLdouble  lineRadius=0.001;
    bool      bSolid=true;
    bool      bBlend=true;
    GLfloat fCurrentColor[4];
    // Get the current color
    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);

    // Save the current transformation matrix..
    glPushMatrix();
    // Create a quadratic object used to draw our axis
    // cylinders and cones
    GLUquadricObj* pQuadric = gluNewQuadric();
    if(!pQuadric)
        return;

    // Set the quadric state
    gluQuadricNormals(pQuadric, GLU_SMOOTH);
    gluQuadricTexture(pQuadric, GL_FALSE);

    if(bSolid)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gluQuadricDrawStyle(pQuadric, GLU_FILL);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        gluQuadricDrawStyle(pQuadric, GLU_LINE);
    }

    // Enable alpha blending?
    if(bBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    //draw grid
    //rotate for x
    glRotated(90, 0.0, 1.0, 0.0);
    for(int i=0; i<sizeY; i++){
        gluCylinder(pQuadric, lineRadius, lineRadius, (double)sizeX/10, iSlices, iStacks);
              glTranslated(0.0f, 0.1f, 0.0f);
    }
              gluCylinder(pQuadric, lineRadius, lineRadius, (double)sizeX/10, iSlices, iStacks);
    glRotated(90, 1.0, 0.0, 0.0);
    for(int i=0; i<sizeX+1; i++){
              gluCylinder(pQuadric, lineRadius, lineRadius, (double)sizeY/10, iSlices, iStacks);
              glTranslated(0.0f, 0.1f, 0.0f);
    }
    // Delete the quadric
    gluDeleteQuadric(pQuadric);

    // Restore the current transformation matrix..
    glPopMatrix();

    // Restore the current color
    glColor4fv(fCurrentColor);

    // Disable blend function
    glDisable(GL_BLEND);
}

//draw axis
void StlView::drawAxis()

{
    GLdouble dAxisLength=1.0;
    GLdouble  dAxisRadius=0.05;
    GLdouble  dArrowLength=0.1;
    GLdouble  dArrowRadius=0.1;
    float     fScale=0.1;
    GLint     iSlices=32;
    GLint     iStacks=32;
    GLfloat   fColorX[4]={1.0,0.0,0.0,1.0};
    GLfloat   fColorY[4]={0.0,1.0,0.0,1.0};
    GLfloat   fColorZ[4]={0.0,0.0,1.0,1.0};
    bool      bSolid=true;
    bool      bBlend=true;
    GLdouble dArrowPosn = dAxisLength;// - (dArrowLength/2);
    GLfloat fCurrentColor[4];

    // Get the current color
    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);

    // Save the current transformation matrix..
    glPushMatrix();

    // Create a quadratic object used to draw our axis
    // cylinders and cones
    GLUquadricObj* pQuadric = gluNewQuadric();
    if(!pQuadric)
        return;

    // Set the quadric state
    gluQuadricNormals(pQuadric, GLU_SMOOTH);
    gluQuadricTexture(pQuadric, GL_FALSE);

    if(bSolid)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gluQuadricDrawStyle(pQuadric, GLU_FILL);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        gluQuadricDrawStyle(pQuadric, GLU_LINE);
    }

    // Enable alpha blending?
    if(bBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // Display a Sphere at the axis origin
    glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    gluSphere(pQuadric, dAxisRadius*fScale*2.5f, iSlices, iStacks);

    // Display the Z-Axis and arrow
    // Set the color
    glColor4fv(fColorZ);

    gluCylinder(pQuadric, dAxisRadius*fScale, dAxisRadius*fScale, dAxisLength*fScale, iSlices, iStacks);
    glTranslated(0.0f, 0.0f, dArrowPosn*fScale);    // Move to arrow position
    gluCylinder(pQuadric, dArrowRadius*fScale, 0.0f, dArrowLength*fScale, iSlices, iStacks);
    gluQuadricOrientation(pQuadric, GLU_INSIDE);
    gluDisk(pQuadric, 0.0f, dArrowRadius*fScale, iSlices, 1);
    gluQuadricOrientation(pQuadric, GLU_OUTSIDE);
    glTranslated(0.0f, 0.0f, -dArrowPosn*fScale);   // Move to 0, 0, 0


    // Display the X-Axis and arrow
    // Set the color
    glColor4fv(fColorX);

    glRotated(90, 0.0, 1.0, 0.0);                   // Rotate for X
    gluCylinder(pQuadric, dAxisRadius*fScale, dAxisRadius*fScale, dAxisLength*fScale, iSlices, iStacks);
    glTranslated(0.0f, 0.0f, dArrowPosn*fScale);    // Move to arrow position
    gluCylinder(pQuadric, dArrowRadius*fScale, 0.0f, dArrowLength*fScale, iSlices, iStacks);
    gluQuadricOrientation(pQuadric, GLU_INSIDE);
    gluDisk(pQuadric, 0.0f, dArrowRadius*fScale, iSlices, 1);
    gluQuadricOrientation(pQuadric, GLU_OUTSIDE);
    glTranslated(0.0f, 0.0f, -dArrowPosn*fScale);   // Move to 0, 0, 0


    // Display the Y-Axis and arrow
    // Set the color
    glColor4fv(fColorY);

    glRotated(-90, 1.0, 0.0, 0.0);                  // Rotate for Y
    gluCylinder(pQuadric, dAxisRadius*fScale, dAxisRadius*fScale, dAxisLength*fScale, iSlices, iStacks);
    glTranslated(0.0f, 0.0f, dArrowPosn*fScale);    // Move to arrow position
    gluCylinder(pQuadric, dArrowRadius*fScale, 0.0f, dArrowLength*fScale, iSlices, iStacks);
    gluQuadricOrientation(pQuadric, GLU_INSIDE);
    gluDisk(pQuadric, 0.0f, dArrowRadius*fScale, iSlices, 1);
    gluQuadricOrientation(pQuadric, GLU_OUTSIDE);
    glTranslated(0.0f, 0.0f, -dArrowPosn*fScale);   // Move to 0, 0, 0


    // Delete the quadric
    gluDeleteQuadric(pQuadric);

    // Restore the current transformation matrix..
    glPopMatrix();

    // Restore the current color
    glColor4fv(fCurrentColor);

    // Disable blend function
    glDisable(GL_BLEND);
}

QString StlView::addObject(QString fileName){
    QString name=QColor(qrand()%256,qrand()%256,qrand()%256).name();
    StlObject *newObject= new StlObject();
    connect(newObject, SIGNAL(progress(int,int,QString)), this, SIGNAL(progress(int,int,QString)));
    connect(newObject, SIGNAL(doneProcessing(bool)), this, SIGNAL(doneProcessing(bool)));
    newObject->loadFile(fileName);
    this->objects.insert(name, newObject);
    if(this->objects.value(name)->getIsManifold()){
        emit nonManifold(name);
    }
    updateGL();
    return name;
}

QString StlView::addObject(StlObject *object){
    QString name=QColor(qrand()%256,qrand()%256,qrand()%256).name();
    qDebug() << object->getFileName();
    StlObject *newObject=new StlObject();
    newObject->copy(object);
    connect(newObject, SIGNAL(progress(int,int,QString)), this, SIGNAL(progress(int,int,QString)));
    connect(newObject, SIGNAL(doneProcessing(bool)), this, SIGNAL(doneProcessing(bool)));
    this->objects.insert(name, newObject);
    if(this->objects.value(name)->getIsManifold()){
        emit nonManifold(name);
    }
    newObject->moveXY(newObject->getOffset().x()+newObject->getWidth()+0.1,newObject->getOffset().y());
    updateGL();
    return name;
}


void StlView::setTableSize(int x, int y){
    this->sizeX=x;
    this->sizeY=y;
    this->xMove=(double)-x/(double)20;
    this->yMove=(double)-y/(double)20;
    updateGL();
}

StlObject* StlView::getObject(QString name){
    return this->objects.value(name);
}

void StlView::removeObject(){
    for(int i=0; i<this->selectedObjects.size(); i++){
        delete this->objects.value(this->selectedObjects.at(i));
        this->objects.remove(this->selectedObjects.at(i));
        this->objectSelected=false;
    }
    this->selectedObjects.clear();
    emit objectPicked(false);
    updateGL();
}

void StlView::mirrorObject(QChar axis){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->mirror(axis);
    }
    updateGL();
}


void StlView::repeairObjectNormals(){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->repairNormals();
    }
    updateGL();
}

void StlView::repeairObjectHoles(){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->repairHoles();
    }
    updateGL();
}

void StlView::rotateObject(double ang){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->rotate(ang);
    }
    updateGL();
}

void StlView::scaleObject(double scale){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->scale(scale);
    }
    updateGL();
}
void StlView::moveObject(QPointF point){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->moveXY(point.x(),point.y());
    }
    updateGL();
}

void StlView::duplicateObject(){
    for(int i=0; i<this->selectedObjects.size(); i++){
        addObject(this->objects.value(this->selectedObjects.at(i)));
    }
    updateGL();
}

QStringList StlView::getColorsList(){
    return this->objects.keys();
}
