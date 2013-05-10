#ifndef SLICEDIALOG_H
#define SLICEDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include "stlview.h"

namespace Ui {
    class SliceDialog;
}

class SliceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SliceDialog(const QGLWidget * shareWidget = 0, QWidget *parent = 0);
    ~SliceDialog();
    void clearObjects();
    void setLastDir(QString dir);
    void setTableSize(int x, int y);

public slots:
    void addObject(QString file="");
    void updateConfigs(QString dir);
    void updateSlicerPath(QString path);
    void updateOutputPath(QString path);
    void updateStlView();
    inline void setMaterialList(QList<Material*>* list) {materials=list; stlView->setMaterialList(list);}
private slots:
    void on_addBtn_clicked();
    void on_sliceBtn_clicked();
    void slicingFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateStatus();
    void updateErrors();
    void setOffset(QPointF);
    void rotateObject(int);
    void scaleObject(int);
    void objectScaled(int);
    void objectRotated(int);
    void moveObjectX(double);
    void moveObjectY(double);
    void update_show_Angles(bool);
    void update_show_Support(bool);

    void nonManifold(QString name);
    void on_exportStlBtn_clicked();
    void on_confCombo_currentIndexChanged(const QString &arg1);

    void updateProgress(int value, int max, QString text);

    void on_selectTool_clicked();

    void on_delTool_clicked();

    void on_moveTool_clicked();

    void on_rotateTool_clicked();

    void on_toolButton_clicked();

    void processingFinished(bool);

    void on_centerTool_clicked();

    void on_cloneTool_clicked();

    void on_mirrorTool_clicked();

    void on_boxTool_clicked();

    void on_repairTool_clicked();

    void on_sliceTool_clicked();

    void on_materialTool_clicked();

    void on_layerSlider_valueChanged(int value);

    void on_showLayers_toggled(bool checked);

    void on_showAngles_toggled(bool checked);

    void on_generateSupport_clicked();

private:
    QList<Material*>* materials;
    Ui::SliceDialog *ui;
    QString lastDir;
    QString slicerPath;
    QString outputPath;
    QString configPath;
    QString outputFile;
    int xSize;
    int ySize;
    QPoint printCenter;
    QProcess *slicerProcess;
    StlView *stlView;
    QString saveStl(QString fileName);
    int lastRot;
signals:
    void fileSliced(QString);
};

#endif // SLICEDIALOG_H
