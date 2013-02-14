#ifndef PRINTER_H
#define PRINTER_H

#include <QObject>
#include <QTime>
#include <QDebug>
#include <QStringList>
#include <QPoint>
#include <QTextCodec>
#include <QThread>
#include <QTimer>
#include <QVector3D>
#include <QMutex>
#include <QWaitCondition>
#include "qextserialport.h"
#include "qextserialenumerator.h"


//thread class for serial port communication, currently not used
class Printer : public QObject
{
    Q_OBJECT
private:
     QextSerialPort *portObj;
     bool isPrinting;
     bool isPaused;
     bool monitorTemperature;
     QTimer *temperatureTimer;
     QTimer *readTimer;
     QVector3D curr_pos;
     QString inBuffer;
     QStringList responseBuffer;
     bool writeNext;
     double last_bed_temp;
     double last_head_temp;
     bool connectionActive;
     QString checkSum(QString command);
     //hold lines that were sent to printer
     QStringList sentLines;
     //hold lines to be send to printer
     QStringList priQuery;
     QStringList mainQuery;
     int lineNum;
     int resendFrom;
     //SD stuff
     QStringList sdFiles;
     bool updatingFileList;
     bool sdPrint;
     bool uploadingToSD;
     QString uploadedFileName;
public:
    explicit Printer(QObject *parent = 0);
    bool isConnected();
    bool isPrintingFromSD();
    bool getIsPrinting();
signals:
    void write_to_console(QString);
    void currentTemp(double,double,double);
    void progress(int,int);
    void uploadProgress(int,int);
    void currentPosition(QVector3D);
    void connected(bool);
    void newResponce(QString);
    void settingTemp1(double);
    void settingTemp3(double);
    void printFinished(bool);
    void clearToSend();
    //SD card stuff
    void SDFileList(QStringList);
private slots:
    void processResponce(QString);
public slots:
    //read
    void readFromPort();
    //write
    int writeToPort(QString command,int lineNum=-1, bool calcCheckSum=false);
    //connecting to port
    bool connectPort(QString port, int baud);
    bool disconnectPort();
    //slots for printer control
    //moving axis
    void homeX();
    void homeY();
    void homeZ();
    void homeAll();
    //SD card stuff
    void getSDFilesList();
    void uploadToSD(QString filename, QStringList data);
    void selectSDFile(QString filename);
    void startResumeSdPrint();
    void pauseSDPrint();
    void set_isSdPrinting(bool);
    //move head x/y
    void moveHead(QPoint point, int speed);
    //move head Z
    void moveHeadZ(double z, int speed,bool relative);
    //setting fan speed
    void setFan(int percent);
    //disable stepper
    void disableSteppers();
    void loadToBuffer(QStringList buffer, bool clear);
    void startPrint();
    void pausePrint(bool);
    void setMonitorTemperature(bool);
    void getTemperature();
    void setTemp1(int);
    void setTemp3(int);
    void extrude(int lenght, int speed);
    void retrackt(int lenght, int speed);
    void send(QString command);
    void send_now(QString command);
    void send_next();


};

#endif // PRINTER_H
