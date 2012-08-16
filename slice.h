#ifndef SLICE_H
#define SLICE_H

#include <QObject>
#include <face.h>
#if defined(Q_WS_MAC)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined(Q_WS_WIN)
#include <GL/gl.h>
#include <GL/glu.h>
#endif

//class for basinc slicing [without gcode generation]

class Slice : public QObject
{
    Q_OBJECT
public:
    explicit Slice(QObject *parent = 0);
    inline void setLayerHeight(double layer){ layerHeight=layer;}
    void fillTriLayer(QHash<QString, Face*> faces);
    void draw(int layer);
private:
    //list of triangles in layes
    QList< QList<Face*>* > triLayer;
    //layer height
    double layerHeight;
signals:

public slots:

};

#endif // SLICE_H
