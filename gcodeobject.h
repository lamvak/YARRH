#ifndef GCODEOBJECT_H
#define GCODEOBJECT_H


#include <QVector3D>
#include <QObject>
#include <QtOpenGL>
#if defined(Q_WS_MAC)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined(Q_WS_WIN)||defined(Q_WS_X11)
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include "layer.h"

class GCodeObject : public QObject
{
    Q_OBJECT
public:
    GCodeObject(QObject *parent);
    ~GCodeObject();
    void addVertex(qreal xpos, qreal ypos, qreal zpos, qreal travel, int layer);
    void draw(float scale, int layers, bool show_travel, int current_layer);
    void setLayerHeight(float layerHeight);
    float getLayerHeight();
    void render(float scale);
    void freeLists();
private:
    QList<Layer*> layerList;
    QMap<int, QVector4D> cylinderList;
    float layerHeight;
    int currentLayer;
};

#endif // GCODEOBJECT_H
