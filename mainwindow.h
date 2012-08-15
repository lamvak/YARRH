#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QTime>
#include "glwidget.h"
#include "gcodeobject.h"
#include "graphwidget.h"
#include "headcontrol.h"
#include "printer.h"
#include "aboutwindow.h"
#include "calibratedialog.h"
#include "optiondialog.h"
#include "slicedialog.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"

//version number
#define VERSION_MAJOR       0
#define VERSION_MINOR       1
#define VERSION_REVISION    8

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *e);
private:
    Ui::MainWindow *ui;
    QString fileContent;
    QStringList gcodeLines;
    QTime startTime;
    QTime durationTime;
    QTime eta;
    QString lastOpendDir;
    QextSerialEnumerator *portEnum;
    //Printer obj
    Printer *printerObj;
    //about window
    AboutWindow *aboutWindow;
    //calibrate dialog
    CalibrateDialog *calibrateDialog;
    //options dialog
    OptionDialog *optionDialog;
    //slice dialog
    SliceDialog *sliceDialog;
    //confirmation dialog
    QMessageBox* msgBox;
    qreal lastZ;
    int currentLayer;
    void saveSettings();
    void restoreSettings();
private slots:
    void on_connectBtn_toggled(bool);
    void loadFile(QString fileName="");
    void on_printBtn_clicked();
    //set layers
    void on_layerScrollBar_valueChanged(int layers);
    void moveHead(QPoint point);
    //pausing print
    void on_pauseBtn_toggled(bool);
    void drawTemp(double, double,double);
    void updateProgress(int);
    //setting temperatures
    void on_t1Btn_toggled(bool);
    void setTemp2(bool);
    void on_hbBtn_toggled(bool);

    void moveZ(int);
    void updateZ(int);
    void on_actionO_Programie_triggered();
    void updateHeadGoToXY(QPoint);
    void updateHeadPosition(QVector3D);
    void on_outLine_returnPressed();
    void on_groupBox_2_toggled(bool arg1);
    void on_fanBtn_toggled(bool checked);
    void on_extrudeBtn_clicked();
    void on_retracktBtn_clicked();
    void printerConnected(bool);
    //setting temp in ui from gcode
    void setTemp1FromGcode(double value);
    void setTemp3FromGcode(double value);
    void on_actionCalibrate_printer_triggered();
    void on_graphGroupBox_toggled(bool arg1);
    void on_actionOptions_triggered();
    void updatadeSize(QVector3D newSize);
    void printFinished(bool);
    void on_fineUpZBtn_clicked();
    void on_fineDownZBtn_clicked();
    void on_homeZ_clicked();
    void on_corseUpZBtn_clicked();
    void on_corseDownZBtn_clicked();
};

#endif // MAINWINDOW_H
