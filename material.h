#ifndef MATERIAL_H
#define MATERIAL_H

#include <QObject>
#include <QColor>

//this class will be storing info about object materials
class Material : public QObject
{
    Q_OBJECT
public:
    explicit Material(QColor color, int extruder, QObject *parent = 0);
    inline QColor getColor() {return color;}
    inline void changeColor(QColor newColor) {color=newColor;}
    inline int getExtruder() {return extruder;}
private:
    QColor color;
    int extruder;
signals:

public slots:

};

#endif // MATERIAL_H
