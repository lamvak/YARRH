#include <QtGui>
#include <QtOpenGL>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>
#include "glwidget.h"


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

//constructor
GlWidget::GlWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
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
    zoom = 2.5;
    xMove = 0;
    yMove = 0;
    currentLayer=0;
    layers = 5000;
    show_travel=false;
    qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
}


//destructor
GlWidget::~GlWidget()
 {
 }

void GlWidget::setCurrentLayer(int layer){
    this->currentLayer=layer;
    this->repaint();
}

//size hints
 QSize GlWidget::minimumSizeHint() const
  {
      return QSize(50, 50);
  }

  QSize GlWidget::sizeHint() const
  {
      return QSize(800, 800);
  }

  //inizalization of gl

  void GlWidget::initializeGL()
   {
       qglClearColor(qtPurple.dark());
       glEnable(GL_DEPTH_TEST);
       glEnable(GL_CULL_FACE);
       glShadeModel(GL_FLAT);
       glEnable(GL_LIGHTING);
       glEnable(GL_LIGHT0);
       glEnable(GL_MULTISAMPLE);
       glEnable(GL_COLOR_MATERIAL);
       static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
       glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
   }
  //resize event
  void GlWidget::resizeGL(int width, int height)
   {
       glViewport(0, 0, (GLint)width, (GLint)height);
       glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
#ifdef QT_OPENGL_ES_1
          glOrthof(-0.5 + zoom , +0.5 - zoom, -0.5 + zoom, +0.5 - zoom, 4.0, 100.0);
#else
          glOrtho(-((float)this->width()/1000*zoom),+((float)this->width()/1000*zoom), -((float)this->height()/1000*zoom), +((float)this->height()/1000*zoom), 4.0, 100.0);
#endif
       glMatrixMode(GL_MODELVIEW);
   }

  void GlWidget::paintGL()
   {
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       glLoadIdentity();
       gluLookAt(eyeX, eyeY, eyeZ, -xMove, -yMove, 0.0, upX, upY, upZ);
       for(int i=0; i<this->objects.size(); i++){
           this->objects.at(i)->draw(0.01,this->layers, show_travel, this->currentLayer);
       }
       drawAxis();
       drawGrid();
   }
//mouse press event
  void GlWidget::mousePressEvent(QMouseEvent *event)
   {
       lastPos = event->pos();
   }

  //mouse move event
  void GlWidget::mouseMoveEvent(QMouseEvent *event)
  {
      int dx = event->x() - lastPos.x();
      int dy = event->y() - lastPos.y();
      if(event->x()>0 && event->y()>0 && event->x()<this->width() && event->y()< this->height()){
          QPointF clicked=screenToWorld(lastPos.x(),lastPos.y());
          QPointF currentClicked=screenToWorld(event->x(),event->y());

          if (event->buttons() & Qt::MiddleButton) {

          } else if (event->buttons() & Qt::LeftButton) {
              // Mouse point to angle conversion
              theta -= dy*1.0;
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

          lastPos = event->pos();
      }
  }
//wheel event
  void GlWidget::wheelEvent(QWheelEvent * event){

      if((zoom-(float)event->delta()/1920)>0){
          zoom -= (float)event->delta()/1920;
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
#ifdef QT_OPENGL_ES_1
          glOrthof(-0.5 + zoom , +0.5 - zoom, -0.5 + zoom, +0.5 - zoom, 4.0, 100.0);
#else
          glOrtho(-((float)this->width()/1000*zoom),+((float)this->width()/1000*zoom), -((float)this->height()/1000*zoom), +((float)this->height()/1000*zoom), 4.0, 100.0);
#endif
          glMatrixMode(GL_MODELVIEW);

          updateGL();
      }
  }


  //drawing 20x20cm grind in 3dview
  void GlWidget::drawGrid(){
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
  void GlWidget::drawAxis()

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


  void GlWidget::addObject(GCodeObject *object){
      this->objects.append(object);
  }
  //clearing object list... not sure if deletes them also, will check later
  void GlWidget::clearObjects(){
      for(int i=0; i<this->objects.size(); i++){
          this->objects.at(i)->freeLists();
          delete this->objects.at(i);
      }
      this->objects.clear();
  }

  void GlWidget::setLayers(int layers){
      this->layers=layers;
      updateGL();
  }

  void GlWidget::showTravel(bool show){
      this->show_travel=show;
      updateGL();
  }

  QPointF GlWidget::screenToWorld(int x, int y){
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
       qDebug() << winX << winY << winZ;
       gluUnProject( winX, winY,  winZ, modelview, projection, viewport, &posX, &posY, &posZ);

     return QPointF(posX,posY);
  }


  void GlWidget::setTableSize(int x, int y){
      this->sizeX=x;
      this->sizeY=y;
      this->xMove=(double)-x/(double)20;
      this->yMove=(double)-y/(double)20;
      updateGL();
  }
