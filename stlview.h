#ifndef STLVIEW_H
#define STLVIEW_H

#include <QGLWidget>
#include <GL/glu.h>
#include <GL/gl.h>
#include <QtOpenGL>
#include <QRect>
#include <btBulletDynamicsCommon.h>
#include "stlobject.h"

class StlView : public QGLWidget
{
    Q_OBJECT
public:
    StlView(QWidget *parent = 0,const QGLWidget * shareWidget = 0);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    QString addObject(QString fileName);
    QString addObject(StlObject *object);
    void setTableSize(int x, int y);
    QStringList getColorsList();
    ///tool enum
    enum tools{
        SELECT = 0,
        MOVE = 1,
        SCALE = 2,
        ROTATE = 3,
        REMOVE = 4,
        CENTER = 5,
        COPY = 6,
        MIRROR = 7,
        REPAIR = 8,
        BOX_SELECT = 9
    };

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
    void repairObject();
    StlObject* getObject(QString);
    void removeObject();
    void rotateObject(double);
    void scaleObject(double);
    void moveObject(QPointF);
    void mirrorObject(QChar axis);
    void duplicateObject();
    void viewTop();
    void viewFront();
    void viewSide();
    void centerCamera();
    void updateBoxCords();
    void clearObjects();
    void setActiveTool(int tool);
    void mirrorX();
    void mirrorY();
    void mirrorZ();
protected:
    void initializeGL();
    void paintGL();
    void paintPicking();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * event);
    void keyPressEvent (QKeyEvent * event);
private:
    QMenu* mirrorMenu;
    QAction* mirrorXAction;
    QAction* mirrorYAction;
    QAction* mirrorZAction;
    int activeTool;
    int previousTool;
    GLfloat eyeX,eyeY,eyeZ;
    GLfloat upX,upY,upZ;
    GLfloat theta,phi;
    GLfloat r;
    int sizeX, sizeY;
    double clickedX, clickedY;
    bool objectSelected;
    //list of selected objets
    QStringList selectedObjects;
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
    QList<QVector3D> getSelectedObjectsCords();
    QList<QVector3D> oryginalCenters;
    double originalScale;
    QList<double> oryginalScales;
    QList<double> getScales();
    bool drawBox;
    QPointF selectionBoxP1;
    QPointF selectionBoxP2;
    QTimer* boxUpdater;
    void drawSelectionBox();
    bool allMirrored();
    //getting ready to use bullet physics engine
//    btCollisionWorld* collisionWorld;
//    btDefaultCollisionConfiguration* collisionConfiguration;
//    btCollisionDispatcher* dispatcher;
};


#endif // STLVIEW_H
