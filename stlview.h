#ifndef STLVIEW_H
#define STLVIEW_H

#include <QGLWidget>

#if defined(Q_WS_MAC)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined(Q_WS_WIN)
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <QtOpenGL>
#include <QRect>
#include "stlobject.h"
#include "material.h"
//#include "slice.h"

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
        SLICE = 9,
        FILL = 10,
        BOX_SELECT = 11
    };
    void setMaterialList(QList<Material*>* list);
    inline void setActiveMaterial(int Material) {activeMaterial=Material;}
signals:
    void objectPicked(bool);
    void selectedRotation(int);
    void selectedScale(int);
    void selectedCors(QPointF);
    void selectedCol(QString);
    void nonManifold(QString);
    void selectedShowAngles(bool);
    void selectedShowSupport(bool);
    void progress(int value, int max, QString text);
    void doneProcessing(bool);
public slots:
    void selectObject(QString);
    void repairObject();
    StlObject* getObject(QString);
    void removeObject();
    void rotateObject(double);
    void changeObjectMaterial();
    void sliceObject();
    void scaleObject(double);
    void moveObject(QPointF);
    void mirrorObject(QChar axis);
    void setShowAngles(bool);
    void setShowSupport(bool);
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
    inline void setShowLayers(bool value) { showLayers=value; updateGL(); }
    void setLayerNum(int value);
    void generateSupport(qreal treshold);
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
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
private:
    bool showLayers;
    int layerNum;
    QMenu* mirrorMenu;
    QAction* mirrorXAction;
    QAction* mirrorYAction;
    QAction* mirrorZAction;
    int activeTool;
    int previousTool;
    int activeMaterial;
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
    QList<Material*>* materials;
    double originalScale;
    QList<double> oryginalScales;
    QList<double> getScales();
    bool drawBox;
    QPointF selectionBoxP1;
    QPointF selectionBoxP2;
    QTimer* boxUpdater;
    void drawSelectionBox();
    bool allMirrored();
};


#endif // STLVIEW_H
