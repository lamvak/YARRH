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
    explicit SliceDialog(QWidget *parent = 0);
    ~SliceDialog();
    void clearObjects();
    void setLastDir(QString dir);
    void setTableSize(int x, int y);
public slots:
    void addObject(QString file="");
    void updateConfigs(QString dir);
    void updateSlicerPath(QString path);
    void updateOutputPath(QString path);   
private slots:
    void on_addBtn_clicked();
    void on_sliceBtn_clicked();
    void slicingFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateStatus();
    void setOffset(QPointF);
    void rotateObject(int);
    void scaleObject(int);
    void objectScaled(int);
    void objectRotated(int);
    void moveObjectX(double);
    void moveObjectY(double);

    void nonManifold(QString name);
    void on_removeBtn_clicked();
    void on_exportStlBtn_clicked();
    void on_confCombo_currentIndexChanged(const QString &arg1);

    void on_mirrorXBtn_clicked();

    void on_mirrorYBtn_clicked();

    void on_mirrorZBtn_clicked();

    void on_repeairNormals_clicked();

    void updateProgress(int value, int max, QString text);
    void on_repairHoles_clicked();

    void on_duplicateBtn_clicked();

    void on_mToCenterBtn_clicked();

private:
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
signals:
    void fileSliced(QString);
};

#endif // SLICEDIALOG_H
