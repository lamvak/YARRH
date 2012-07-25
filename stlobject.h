#ifndef STLOBJECT_H
#define STLOBJECT_H

#include <QObject>
#include <QtOpenGL>
#include <GL/glut.h>
#include <GL/gl.h>
#include <QVector3D>
#include <QDebug>
#include <cmath>

class StlObject : public QObject
{
    Q_OBJECT
private:
    double scaleValue;
    double rotation;
    QString nameString;
    QString fileName;
    QVector3D offset;
    bool selected;
    bool colliding;
    float width;
    float lenght;
    float height;
    int name;
    QList<QVector3D> vertexes;
    QVector3D ComputeFaceNormal(QVector3D p1, QVector3D p2, QVector3D p3);
    QVector3D VectorGetNormal(QVector3D a, QVector3D b);
    QVector3D VectorOffset(QVector3D pIn, QVector3D pOffset);
public:
    explicit StlObject(QString fileName, int name, QObject *parent = 0);
    void draw(bool);
    void select(bool);
    void moveXY(double x, double y);
    void rotate(double angle);
    void scale(double value);
    QVector3D getOffset();
    QList<QVector3D> transform();
    QList<QVector3D> getTriangles();
    double getRotation();
    double getScale();
    void mirror(QChar axis);
signals:

public slots:

};

#endif // STLOBJECT_H
