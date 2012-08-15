#include <QtGui>
#include <QtOpenGL>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>
#include "stlview.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

StlView::StlView(QWidget *parent,const QGLWidget * shareWidget)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent, shareWidget)
{
    zoom = 2.5;
    xMove = 0;
    yMove = 0;
    clickedX=0.0;
    clickedY=0.0;
    theta=145;
    phi=117;
    r=9.8;
    eyeX=3.64;
    eyeY=-7.15;
    eyeZ=5.62;
    upX=-0.025;
    upY=0.05;
    upZ=0.079;
    sizeX=20;
    sizeY=20;
    drawBox=false;
    qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
    objectSelected = false;
    this->setFocusPolicy(Qt::StrongFocus);
    this->boxUpdater=new QTimer();
    connect(this->boxUpdater, SIGNAL(timeout()), this, SLOT(updateBoxCords()));
    setActiveTool(MOVE);
    //create mirror menu
    this->mirrorMenu = new QMenu(tr("Mirror"),this);
    this->mirrorXAction =  this->mirrorMenu->addAction(tr("Mirror X"));
    connect(this->mirrorXAction, SIGNAL(triggered(bool)), this, SLOT(mirrorX()));
    this->mirrorYAction =  this->mirrorMenu->addAction(tr("Mirror Y"));
    connect(this->mirrorYAction, SIGNAL(triggered(bool)), this, SLOT(mirrorY()));
    this->mirrorZAction =  this->mirrorMenu->addAction(tr("Mirror Z"));
    connect(this->mirrorZAction, SIGNAL(triggered(bool)), this, SLOT(mirrorZ()));
}

void StlView::setMaterialList(QList<Material *> *list){
    QHash<QString, StlObject*>::iterator j;
    for(j = this->objects.begin(); j != this->objects.end(); ++j){
        if(!list->contains(j.value()->getMaterial())){
            j.value()->setMaterial(list->first());
        }
    }
    this->materials=list;
    this->activeMaterial=0;
    updateGL();
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

    if((event->buttons() & Qt::LeftButton)){
        //if we clicked on object
        if(this->activeTool==BOX_SELECT){
            this->drawBox=true;
            this->selectionBoxP1=screenToWorld(this->mapFromGlobal(QCursor::pos()).x(),this->mapFromGlobal(QCursor::pos()).y());
            this->selectionBoxP2=this->selectionBoxP1;
        }
        if(pickedColor.name()!="#4e4e7f" && this->objects.keys().contains(pickedColor.name())){
            if(this->activeTool==SELECT || this->activeTool==BOX_SELECT){
                if(event->modifiers()!=Qt::ControlModifier){
                    for(int i=0; i<this->selectedObjects.size(); i++){
                        if(pickedColor.name()!=this->selectedObjects.at(i))
                            this->objects.value(this->selectedObjects.at(i))->select(false);
                    }
                }
                if(this->selectedObjects.size()>1 && event->modifiers()!=Qt::ControlModifier){
                    this->objects.value(pickedColor.name())->select(false);
                }
                this->selectedObjects.clear();

                if(this->objects.value(pickedColor.name())->isSelected()){
                    this->objects.value(pickedColor.name())->select(false);
                }
                else{
                    this->objects.value(pickedColor.name())->select(true);
                }
                QHash<QString, StlObject*>::iterator j;
                for(j = this->objects.begin(); j != this->objects.end(); ++j){
                    if(j.value()->isSelected()){
                        this->selectedObjects.append(j.key());
                    }
                }
            }
            else{
                if(this->activeTool!=BOX_SELECT){
                    if(this->selectedObjects.contains(pickedColor.name()) && event->modifiers()!=Qt::ControlModifier)
                    {
                        if(this->activeTool==REMOVE){
                            removeObject();
                        }
                        if(this->activeTool==CENTER){
                            moveObject(QPointF((float)this->sizeX/20,(float)this->sizeY/20));
                        }
                        if(this->activeTool==COPY){
                            duplicateObject();
                        }
                        if(this->activeTool==REPAIR){
                            repairObject();
                        }
                        if(this->activeTool==SLICE){
                            sliceObject();
                        }
                        if(this->activeTool==FILL){
                            changeObjectMaterial();
                        }
                        if(this->activeTool==MIRROR){
                            this->mirrorMenu->move(event->globalPos());
                            this->mirrorMenu->show();
                        }
                    }
                    else{
                        if(event->modifiers()==Qt::ControlModifier){
                            if(this->selectedObjects.contains(pickedColor.name())){
                                this->objects.value(pickedColor.name())->select(false);
                                this->selectedObjects.removeAt(this->selectedObjects.indexOf(pickedColor.name()));
                            }
                            else{
                                this->selectedObjects.append(pickedColor.name());
                                this->objects.value(pickedColor.name())->select(true);
                            }
                        }
                        else{
                            if(!this->selectedObjects.contains(pickedColor.name())){
                                for(int i=0; i<this->selectedObjects.size(); i++){
                                    if(pickedColor.name()!=this->selectedObjects.at(i))
                                        this->objects.value(this->selectedObjects.at(i))->select(false);
                                }
                                this->selectedObjects.clear();
                                this->objects.value(pickedColor.name())->select(true);
                                this->selectedObjects.append(pickedColor.name());
                            }
                        }
                    }
                }
            }
        }
    }
    if(this->selectedObjects.contains(pickedColor.name())){
        this->objectSelected=true;
    }
    else{
       this->objectSelected=false;
    }
    updateGL();
    emit objectPicked(this->objectSelected);

    this->oryginalCenters=getSelectedObjectsCords();

    if(this->objectSelected && this->selectedObjects.size()>0){
        emit selectedCol(this->selectedObjects.last());
        emit selectedRotation(((((int)this->objects.value(this->selectedObjects.last())->getRotation()/5)*5)%360)*-1);
        if(this->selectedObjects.size()==1){
            emit selectedScale((int)(this->objects.value(this->selectedObjects.last())->getScale()*100));
            this->originalScale=this->objects.value(this->selectedObjects.last())->getScale();
        }
        else{
            emit selectedScale(100);
            this->originalScale=1.0;
            this->oryginalScales=getScales();
        }
        emit selectedCors(getSelectedObjectsCords().at(0).toPointF());
    }
}

void StlView::selectObject(QString name){
    //deselect old objects
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
    if(this->activeTool==BOX_SELECT){
        this->drawBox=false;
        updateGL();
    }
    this->oryginalCenters=getSelectedObjectsCords();
}

//mouse move event
void StlView::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    if(event->x()>0 && event->y()>0 && event->x()<this->width() && event->y()< this->height()){
        QPointF clicked=screenToWorld(lastPos.x(),lastPos.y());
        QPointF currentClicked=screenToWorld(event->x(),event->y());
        QLineF start(getSelectedObjectsCords().at(0).toPointF().x(),getSelectedObjectsCords().at(0).toPointF().y(),clicked.x(),clicked.y());
        QLineF current(getSelectedObjectsCords().at(0).toPointF().x(),getSelectedObjectsCords().at(0).toPointF().y(),currentClicked.x(),currentClicked.y());

        //            if(event->buttons() & Qt::RightButton){
        //                this->originalScale+=(double)dx*0.01;
        //                scaleObject(this->originalScale);
        //            }
        //left selects, or uses choosen tool
        if(event->buttons() & Qt::LeftButton && this->drawBox && this->activeTool==BOX_SELECT){
            updateBoxCords();
        }
        if(event->buttons() & Qt::LeftButton && this->objectSelected){
            switch(this->activeTool){
            case ROTATE:
                rotateObject((start.angleTo(current)>180 ? -start.angleTo(current)-360:-start.angleTo(current)));
                break;
            case SCALE:
                this->originalScale+=(double)dx*0.01;
                scaleObject(this->originalScale);
                break;
            case MOVE:
                for(int i=0; i<this->selectedObjects.size(); i++){
                    this->objects.value(this->selectedObjects.at(i))->moveXY(this->objects.value(this->selectedObjects.at(i))->getOffset().x()+currentClicked.x()-clicked.x(),this->objects.value(this->selectedObjects.at(i))->getOffset().y()+currentClicked.y()-clicked.y());
                }
                if(this->selectedObjects.size()>0)
                    emit selectedCors(getSelectedObjectsCords().at(0).toPointF());
                break;
            }
        }
        //middle button move objects
        if((event->buttons() & Qt::MiddleButton) && (event->modifiers()!=Qt::ShiftModifier) && this->objectSelected){
            for(int i=0; i<this->selectedObjects.size(); i++){
                this->objects.value(this->selectedObjects.at(i))->moveXY(this->objects.value(this->selectedObjects.at(i))->getOffset().x()+currentClicked.x()-clicked.x(),this->objects.value(this->selectedObjects.at(i))->getOffset().y()+currentClicked.y()-clicked.y());
            }
            if(this->selectedObjects.size()>0)
                emit selectedCors(getSelectedObjectsCords().at(0).toPointF());
        }
        if ((event->buttons() & Qt::RightButton) && (event->modifiers()!=Qt::ShiftModifier)) {
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
        }
        if ((event->buttons() & Qt::RightButton) && (event->modifiers()==Qt::ShiftModifier)){
            xMove -= clicked.x()-currentClicked.x();
            yMove -= clicked.y()-currentClicked.y();
        }

        lastPos = event->pos();
    }
    updateGL();
}

void StlView::keyPressEvent(QKeyEvent * event){
    qDebug() << event->key();
    switch(event->key()){
    case 49:
        viewFront();
        break;
    case 51:
        viewSide();
        break;
    case 55:
        viewTop();
        break;
    case 53:
        centerCamera();
        break;
    case 68:
        setActiveTool(COPY);
        break;
    case 82:
        setActiveTool(ROTATE);
        break;
    case 71:
        setActiveTool(MOVE);
        break;
    case 77:
        setActiveTool(MIRROR);
        break;
    case 83:
        setActiveTool(SCALE);
        break;
    case 16777223:
        setActiveTool(REMOVE);
        break;
    case 66:
        setActiveTool(BOX_SELECT);
        updateGL();
        break;
    }
}

void StlView::updateBoxCords(){
    this->selectionBoxP2=screenToWorld(this->mapFromGlobal(QCursor::pos()).x(),this->mapFromGlobal(QCursor::pos()).y());
    QHash<QString, StlObject*>::iterator j;
    QRectF rect(this->selectionBoxP1,this->selectionBoxP2);
    for(j = this->objects.begin(); j != this->objects.end(); ++j){
        if(rect.contains(j.value()->getOffset().toPointF())){
            if(!this->selectedObjects.contains(j.key())){
                j.value()->select(true);
                if(j.value()->isMirrored()){
                    this->selectedObjects.prepend(j.key());
                }
                else{
                    this->selectedObjects.append(j.key());
                }

            }
        }
        else{
            j.value()->select(false);
            this->selectedObjects.removeAt(this->selectedObjects.indexOf(j.key()));
        }
    }
    updateGL();
}

//wheel event
void StlView::wheelEvent(QWheelEvent * event){

    if((r-(float)event->delta()/100)>0){
        r -= (float)event->delta()/200;
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
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
//    GLfloat specular[] = {1.0f, 1.0f, 1.0f , 1.0f};
//    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    GLfloat whiteSpecularLight[] = {0.5, 0.5, 0.5};
    GLfloat blackAmbientLight[] = {0.8, 0.8, 0.8};
    GLfloat whiteDiffuseLight[] = {1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_SPECULAR, whiteSpecularLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, blackAmbientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuseLight);
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

    if(this->drawBox){
        drawSelectionBox();
    }
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

void StlView::drawSelectionBox(){
    GLfloat fCurrentColor[4];
    // Get the current color
    glGetFloatv(GL_CURRENT_COLOR, fCurrentColor);
    GLfloat   color[4]={1.0,0.0,0.0,1.0};

    glColor4fv(color);
    glLineWidth(2.0);
    glPushAttrib(GL_ENABLE_BIT);
    glLineStipple(1, 0x0F0F);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
        glVertex3f(this->selectionBoxP1.x(), this->selectionBoxP1.y(), 0);
        glVertex3f(this->selectionBoxP2.x(), this->selectionBoxP1.y(), 0);

        glVertex3f(this->selectionBoxP1.x(), this->selectionBoxP1.y(), 0);
        glVertex3f(this->selectionBoxP1.x(), this->selectionBoxP2.y(), 0);

        glVertex3f(this->selectionBoxP1.x(), this->selectionBoxP2.y(), 0);
        glVertex3f(this->selectionBoxP2.x(), this->selectionBoxP2.y(), 0);

        glVertex3f(this->selectionBoxP2.x(), this->selectionBoxP1.y(), 0);
        glVertex3f(this->selectionBoxP2.x(), this->selectionBoxP2.y(), 0);

    glEnd( );
    glPopAttrib();
    // Restore the current color
    glColor4fv(fCurrentColor);
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
    newObject->setMaterial(materials->first());
    this->objects.insert(name, newObject);
    if(this->objects.value(name)->getIsManifold()){
        emit nonManifold(name);
    }
    updateGL();
    return name;
}

QString StlView::addObject(StlObject *object){
    QString name=QColor(qrand()%256,qrand()%256,qrand()%256).name();
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
    QList<QVector3D> centers=getSelectedObjectsCords();
    QMatrix4x4 matrix;
    matrix.scale(1.0,1.0,1.0);
    if(axis=='x'){
        matrix.scale(-1.0,1.0,1.0);
    }
    if(axis=='y'){
        matrix.scale(1.0,-1.0,1.0);
    }
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->mirror(axis);
        centers.replace(i+1,centers.at(i+1)*matrix);
        this->objects.value(this->selectedObjects.at(i))->moveXY(centers.at(i+1).x()+centers.at(0).x(),centers.at(i+1).y()+centers.at(0).y());
    }
}

void StlView::mirrorX(){
    mirrorObject('x');
}

void StlView::mirrorY(){
    mirrorObject('y');
}

void StlView::mirrorZ(){
    mirrorObject('z');
}

void StlView::repairObject(){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->repairNormals();
    }
//    for(int i=0; i<this->selectedObjects.size(); i++){
//        this->objects.value(this->selectedObjects.at(i))->repairHoles();
//    }
}

void StlView::rotateObject(double ang){
    QMatrix4x4 matrix;
    QList<QVector3D> centers=getSelectedObjectsCords();
    double angleDiff=this->objects.value(this->selectedObjects.last())->getRotation();
    for(int i=0; i<this->selectedObjects.size(); i++){
            this->objects.value(this->selectedObjects.at(i))->rotate(ang);
    }
    angleDiff-=this->objects.value(this->selectedObjects.last())->getRotation();
    if(this->objects.value(this->selectedObjects.last())->isMirrored()){
        matrix.rotate(-angleDiff,0,0,1);
    }
    else{
        matrix.rotate(angleDiff,0,0,1);
    }

    for(int i=0; i<this->selectedObjects.size(); i++){
        centers.replace(i+1, centers.at(i+1)*matrix);
        this->objects.value(this->selectedObjects.at(i))->moveXY(centers.at(i+1).x()+centers.at(0).x(),centers.at(i+1).y()+centers.at(0).y());
    }
    if(this->selectedObjects.size()>0)
        emit selectedRotation((((int)this->objects.value(this->selectedObjects.last())->getRotation())%360)*-1);
    updateGL();
}

void StlView::scaleObject(double scale){
    QVector3D temp;
    QMatrix4x4 matrix;
    matrix.scale(scale);

    if(this->selectedObjects.size()==1){
        this->objects.value(this->selectedObjects.last())->scale(scale);
    }
    else{
        for(int i=0; i<this->selectedObjects.size(); i++){
            this->objects.value(this->selectedObjects.at(i))->scale(this->oryginalScales.at(i)*scale);
        }
    }
    for(int i=0; i<this->selectedObjects.size(); i++){
        temp=this->oryginalCenters.at(i+1)*matrix;
        this->objects.value(this->selectedObjects.at(i))->moveXY(temp.x()+this->oryginalCenters.at(0).x(),temp.y()+this->oryginalCenters.at(0).y());
    }
    if(this->selectedObjects.size()>0)
        emit selectedScale((int)(scale*1000)/10);
    updateGL();
}


void StlView::moveObject(QPointF point){
    QList<QVector3D> centers=getSelectedObjectsCords();
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->moveXY(point.x()+centers.at(i+1).x(),point.y()+centers.at(i+1).y());
    }
    updateGL();
}

void StlView::duplicateObject(){
    QStringList oldObjects=this->selectedObjects;
    QString newObject;
    for(int i=0; i<oldObjects.size(); i++){
        newObject=addObject(this->objects.value(oldObjects.at(i)));
        this->objects.value(oldObjects.at(i))->select(false);
        this->selectedObjects.removeAt(0);
        this->selectedObjects.append(newObject);
        this->objects.value(newObject)->select(true);
    }
    updateGL();
}

void StlView::changeObjectMaterial(){
    for(int i=0; i<this->selectedObjects.size(); i++){
        this->objects.value(this->selectedObjects.at(i))->setMaterial(this->materials->at(this->activeMaterial));
    }
}

//here will be basic slicing for layer view
void StlView::sliceObject(){
    qDebug() << "Ciach!";
}

QStringList StlView::getColorsList(){
    return this->objects.keys();
}

QList<QVector3D> StlView::getSelectedObjectsCords(){
    QList<QVector3D> out;
    qreal x=0;
    qreal y=0;
    for(int i=0; i<this->selectedObjects.size(); i++){
        x+=this->objects.value(this->selectedObjects.at(i))->getOffset().x();
        y+=this->objects.value(this->selectedObjects.at(i))->getOffset().y();
    }
    x=x/this->selectedObjects.size();
    y=y/this->selectedObjects.size();

    out.prepend(QVector3D(x,y,0));
    for(int i=0; i<this->selectedObjects.size(); i++){
        x+=this->objects.value(this->selectedObjects.at(i))->getOffset().x();
        y+=this->objects.value(this->selectedObjects.at(i))->getOffset().y();
        out.append((this->objects.value(this->selectedObjects.at(i))->getOffset()-out.at(0)));
    }

    return out;
}

QList<double> StlView::getScales(){
   QList<double> out;
   for(int i=0; i<this->selectedObjects.size(); i++){
       out.append(this->objects.value(this->selectedObjects.at(i))->getScale());
   }
   return out;
}


void StlView::viewFront(){
    eyeX = 1.0;
    eyeY = -r;
    eyeZ = 0.0;
    upX = 0;
    upY = 0;
    upZ = 1;
    theta = 180;
    phi = 90;
    updateGL();
}
void StlView::viewSide(){
    eyeX = r;
    eyeY = 0;
    eyeZ = 0;
    upX = 0;
    upY = 0;
    upZ = 1;
    theta = 180;
    phi = 180;
    updateGL();
}
void StlView::viewTop(){
    eyeX = 0;
    eyeY = 0;
    eyeZ = r;
    upX = 0;
    upY = 1;
    upZ = 0;
    theta = 90;
    phi = 90;
    updateGL();
}

void StlView::centerCamera(){
    if(this->selectedObjects.size()>0){
        xMove=-getSelectedObjectsCords().at(0).x();
        yMove=-getSelectedObjectsCords().at(0).y();
    }
    else{
        this->xMove=(double)-this->sizeX/(double)20;
        this->yMove=(double)-this->sizeY/(double)20;
    }
    updateGL();
}

bool StlView::allMirrored(){
    bool out=true;
    for(int i=0; i<this->selectedObjects.size(); i++){
        out=out&this->objects.value(this->selectedObjects.at(i))->isMirrored();
    }
    return out;
}

void  StlView::clearObjects(){
    QHash<QString, StlObject*>::iterator j;
    for(j = this->objects.begin(); j != this->objects.end(); ++j){
        j.value()->freeList();
        delete j.value();
    }
    this->objects.clear();
}

void StlView::setActiveTool(int tool){
    this->previousTool=this->activeTool;
    this->activeTool=tool;
    switch(this->activeTool){
    case MOVE:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/drag.png")));
        break;
    case ROTATE:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/rotate.png")));
        break;
    case SELECT:
        this->setCursor(Qt::ArrowCursor);
        break;
    case REMOVE:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/delete.png")));
        break;
    case SCALE:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/scale.png").scaled(16,16)));
        break;
    case COPY:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/stamp.png")));
        break;
    case MIRROR:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/mirror.png").scaled(16,16)));
        break;
    case REPAIR:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/repair.png")));
        break;
    case BOX_SELECT:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/boxselect.png")));
        break;
    case CENTER:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/center.png").scaled(16,16)));
        break;
    case SLICE:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/slice.png")));
        break;
    case FILL:
        this->setCursor(QCursor(QPixmap(":/imgs/icons/fill.png")));
        break;
    }
}
