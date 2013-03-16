//#ifndef SLICE_H
//#define SLICE_H

//#include <QObject>
//#include <face.h>
//#include <halfedge.h>
//#if defined(Q_WS_MAC)
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
//#elif defined(Q_WS_WIN)||defined(Q_WS_X11)
//#include <GL/gl.h>
//#include <GL/glu.h>
//#endif
//#include "clipper/clipper.hpp"
//#ifndef CALLBACK
//#define CALLBACK
//#endif

////class for basic slicing [without gcode generation]
//using namespace ClipperLib;
//typedef void (*TessFuncPtr)();

//class Slice : public QObject
//{
//    Q_OBJECT
//public:
//    explicit Slice(QObject *parent = 0);
//    inline void setLayerHeight(double layer){ layerHeight=layer;}
//    void fillTriLayer(QHash<QString, Face*> faces);
//    void fillEdgeLayer();
//    void fillPointLayer();
//    void fillPolygonLayer();
//    void draw(int layer);
//private:
//    //list of triangles in layes
//    QList< QList<Face*>* > triLayer;
//    QList< QList< QList<HalfEdge*> *>* > edgeLayer;
//    QList< QList< QList<QVector3D> *>* > pointLayer;
//    QList<ClipperLib::Polygons> islands;
//    QList<ClipperLib::Polygons> holes;
//    QList<ClipperLib::Polygons> polygonLayer;
//    QList<GLdouble *> vectors;
//    //layer height
//    double layerHeight;
//    //line plane intersection
//    QVector3D linePLaneIntersection(HalfEdge edge, QVector3D planeOrigin, QVector3D planeNormal);
//    void drawPolygon(Polygons &pgs, int layer);
//    //callbacks
//    //    void CALLBACK beginCallback(GLenum which);
//    //    void CALLBACK errorCallback(GLenum errorCode);
//    //    void CALLBACK endCallback(void);
//    //    void CALLBACK combineCallback(GLdouble coords[3], GLdouble *data[4], GLfloat weight[4], GLdouble **dataOut );
//signals:

//public slots:

//};

//#endif // SLICE_H
