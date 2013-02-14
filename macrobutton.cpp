#include "macrobutton.h"

MacroButton::MacroButton(QWidget *parent) :
    QPushButton(parent)
{
}

MacroButton::MacroButton(QString name, QString value,QWidget *parent) :
    QPushButton(parent)
{
    this->name=name;
    this->value=value;
    this->setText(name);
}
