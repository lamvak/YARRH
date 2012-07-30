#ifndef VERTEX_H
#define VERTEX_H

#include <QVector3D>

class Vertex : public QObject, public QVector3D
{
    Q_OBJECT
public:
    explicit Vertex(Vertex *parent = 0);
    explicit Vertex(qreal xpos, qreal ypos, qreal zpos);
    Vertex & operator=(const Vertex & other);
    Vertex & operator=(const QVector3D & other);
signals:

public slots:

};

#endif // VERTEX_H
