#ifndef STLVIEW_H
#define STLVIEW_H

#include <QGLWidget>
#include <GL/glu.h>
#include <GL/gl.h>
#include <QtOpenGL>
#include <btBulletDynamicsCommon.h>
#include "stlobject.h"

class StlView : public QGLWidget
{
    Q_OBJECT
public:
    StlView(QWidget *parent);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    QString addObject(QString fileName);
    void setTableSize(int x, int y);
signals:
    void objectPicked(bool);
    void selectedRotation(int);
    void selectedScale(int);
    void selectedCors(QPointF);
    void selectedCol(QString);
    void nonManifold(QString);
    void progress(int value, int max, QString text);
    void doneProcessing(bool);
public slots:
    void selectObject(QString);
    void repeairObjectNormals(QString);
    void repeairObjectHoles(QString);
    StlObject* getObject(QString);
    void removeObject(QString);
    void rotateObject(QString, double);
    void scaleObject(QString, double);
    void moveObject(QString, QPointF);
    void mirrorObject(QString name, QChar axis);
protected:
    void initializeGL();
    void paintGL();
    void paintPicking();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * event);
private:
    GLfloat eyeX,eyeY,eyeZ;
    GLfloat upX,upY,upZ;
    GLfloat theta,phi;
    GLfloat r;
    int sizeX, sizeY;
    double clickedX, clickedY;
    bool objectSelected;
    QString object;
    QColor objectColor;
    float xMove;
    float yMove;
    float zoom;
    QPoint lastPos;
    QPointF lastPosWorld;
    QPointF lastObjectPos;
    QColor qtGreen;
    QColor qtPurple;
    void drawAxis();
    void drawGrid();
    QHash<QString, StlObject*> objects;

    QPointF screenToWorld(int x, int y);
    //getting ready to use bullet physics engine
//    btCollisionWorld* collisionWorld;
//    btDefaultCollisionConfiguration* collisionConfiguration;
//    btCollisionDispatcher* dispatcher;
};


#endif // STLVIEW_H
