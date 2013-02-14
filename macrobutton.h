#ifndef MACROBUTTON_H
#define MACROBUTTON_H

#include <QPushButton>

class MacroButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MacroButton(QWidget *parent = 0);
    MacroButton(QString name, QString value,QWidget *parent = 0);
    QString getName() { return name;}
    QString getValue() { return value;}
    void setName(QString new_name) { name=new_name; this->setText(name);}
    void setValue(QString new_value) {value=new_value;}
private:
    QString name;
    QString value;
signals:

public slots:

};

#endif // MACROBUTTON_H
