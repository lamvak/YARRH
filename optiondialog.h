#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QComboBox>
#include <QVector3D>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QColorDialog>
#include <material.h>

namespace Ui {
    class OptionDialog;
}

class OptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionDialog(QWidget *parent = 0);
    ~OptionDialog();
    QString getSlicerDir();
    QString getConfigDir();
    QString getOutputDir();
    QVector3D getSize();
    QList< QPair<int, QString > > getExtruders();
    void addExtruder(QPair<int, QString>);
public slots:
    void setSlicerDir(QString dir);
    void setConfigDir(QString dir);
    void setOutputDir(QString dir);
    void setSize(QVector3D size);
private slots:
    void on_closeBtn_clicked();
    void on_slicerDirBtn_clicked();
    void on_configDirBtn_clicked();
    void on_outputDirBtn_clicked();
    void on_extruderNum_valueChanged(int arg1);

    void on_tableWidget_cellDoubleClicked(int row, int column);

signals:
    void slicerPathChanged(QString);
    void outputPathChanged(QString);
    void newSize(QVector3D);
    void newList(QList<Material*>*);
private:
    Ui::OptionDialog *ui;
    QList<Material*>* materials;
};

#endif // OPTIONDIALOG_H
