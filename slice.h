#ifndef SLICE_H
#define SLICE_H

#include <QObject>
#include <face.h>


//class for basinc slicing [without gcode generation]

class Slice : public QObject
{
    Q_OBJECT
public:
    explicit Slice(QObject *parent = 0);
    inline void setLayerHeight(double layer){ layerHeight=layer;}
    void fillTriLayer(QList<Face*> faces);
private:
    //list of triangles in layes
    QVector< QList<Face*> > triLayer;
    //layer height
    double layerHeight;
signals:

public slots:

};

#endif // SLICE_H
