#ifndef LAYER_H
#define LAYER_H

#include <QObject>
#include <QVector4D>
#include <QVector3D>
#include <QtOpenGL>

#if defined(Q_WS_MAC)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined(Q_WS_QWS)
#include <GL/gl.h>
#include <GL/glu.h>
#endif

//class that hold info about layer in opengl lists for faster rendering
class Layer : public QObject
{
    Q_OBJECT
private:
    int layer_num;
    int list_index;
    void renderLine(QVector3D v1, QVector3D v2, qreal radius, qreal travel, bool last);
    QVector3D calculateNormal(QVector3D v1, QVector3D v2, QVector3D v3);
public:
    explicit Layer(int layer, QObject *parent = 0);
    QList<QVector4D> vertexes;
    void draw(float scale);
    //generate lists
    void render(float scale);
    void display(float scale, bool last, bool show_travel, int current_layer);
    void freeLists();
signals:

public slots:

};

#endif // LAYER_H
