#include "material.h"

Material::Material(QColor color, int extruder, QObject *parent) :
    QObject(parent)
{
    this->color=color;
    this->extruder=extruder;
}
