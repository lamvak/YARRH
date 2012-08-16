#include "calibratedialog.h"
#include "ui_calibratedialog.h"

CalibrateDialog::CalibrateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrateDialog)
{
    ui->setupUi(this);
    ui->EGroup->hide();
}

CalibrateDialog::~CalibrateDialog()
{
    delete ui;
}

QVector4D CalibrateDialog::getCallibrationsSetting(){
    QVector4D out(ui->xCurrentSteps->text().toDouble(),ui->yCurrentSteps->text().toDouble(),ui->zCurrentSteps->text().toDouble(),ui->eCurrentSteps->text().toDouble());
    return out;
}


bool CalibrateDialog::autoCalibrateX(){
    return ui->XGroup->isChecked();
}
bool CalibrateDialog::autoCalibrateY(){
    return ui->YGroup->isChecked();
}
bool CalibrateDialog::autoCalibrateZ(){
    return ui->ZGroup->isChecked();
}
bool CalibrateDialog::autoCalibrateE(){
    return ui->EGroup->isChecked();
}

void CalibrateDialog::setAutoCalibrateX(bool calibrate){
    return ui->XGroup->setChecked(calibrate);
}
void CalibrateDialog::setAutoCalibrateY(bool calibrate){
    return ui->YGroup->setChecked(calibrate);
}
void CalibrateDialog::setAutoCalibrateZ(bool calibrate){
    return ui->ZGroup->setChecked(calibrate);
}
void CalibrateDialog::setAutoCalibrateE(bool calibrate){
    return ui->EGroup->setChecked(calibrate);
}


void CalibrateDialog::setXStepsPerMm(double value){
    ui->xCurrentSteps->setText(QString::number(value));
}
void CalibrateDialog::setYStepsPerMm(double value){
    ui->yCurrentSteps->setText(QString::number(value));
}
void CalibrateDialog::setZStepsPerMm(double value){
    ui->zCurrentSteps->setText(QString::number(value));
}
void CalibrateDialog::setEStepsPerMm(double value){
    ui->eCurrentSteps->setText(QString::number(value));
}

void CalibrateDialog::on_calibrateXBtn_clicked()
{
    bool okM1=false;
    bool okM2=false;
    emit writeToPrinter("G28 X0");
    emit writeToPrinter("M92 X"+ui->xCurrentSteps->text());
    double currentStepsPerMm = ui->xCurrentSteps->text().toDouble();
    double newStepsPerMm = ui->xCurrentSteps->text().toDouble();
    double calibrationDistance = 50;
    double firstMeasurment = QInputDialog::getDouble(this, tr("Input first measurment"),
                                       tr("Measurment"), 0, 0, 10000, 2, &okM1);
    if (okM1){
        emit writeToPrinter("G1 X"+QString::number(calibrationDistance)+" F1000");
        double secondMeasurment = QInputDialog::getDouble(this, tr("Input second measurment"),
                                           tr("Measurment"), calibrationDistance, 0, 10000, 2, &okM1);
        if(okM2){
            newStepsPerMm=(calibrationDistance/qAbs((secondMeasurment-firstMeasurment)))*currentStepsPerMm;
            ui->xCurrentSteps->setText(QString::number(newStepsPerMm));
            emit writeToPrinter("M92 X"+ui->xCurrentSteps->text());
            double error =qAbs((double)1-(calibrationDistance/(secondMeasurment-firstMeasurment)));
            if(error>0.01){

                QMessageBox *msgBox = new QMessageBox(this);
                msgBox->setWindowTitle(tr("Error"));
                msgBox->setText(tr("Error is %1 %, continue callibrating?").arg(QString::number(error*100)));

                QPushButton *yesButton = msgBox->addButton(tr("Yes"), QMessageBox::ActionRole);
                msgBox->addButton(tr("No"), QMessageBox::ActionRole);
                msgBox->exec();

                //If user clicks 'Yes'  button , accept QCloseEvent (Close Window)
                if ((QPushButton*)msgBox->clickedButton() == yesButton)
                {
                   on_calibrateXBtn_clicked();
                }
            }
        }
    }
}

void CalibrateDialog::on_calibrateYBtn_clicked()
{
    bool okM1=false;
    bool okM2=false;
    emit writeToPrinter("G28 Y0");
    emit writeToPrinter("M92 Y"+ui->yCurrentSteps->text());
    double currentStepsPerMm = ui->yCurrentSteps->text().toDouble();
    double newStepsPerMm = ui->yCurrentSteps->text().toDouble();
    double calibrationDistance = 50;
    double firstMeasurment = QInputDialog::getDouble(this, tr("Input first measurment"),
                                       tr("Measurment"), 0, 0, 10000, 2, &okM1);
    if (okM1){
        emit writeToPrinter("G1 Y"+QString::number(calibrationDistance)+" F1000");
        double secondMeasurment = QInputDialog::getDouble(this, tr("Input second measurment"),
                                           tr("Measurment"), calibrationDistance, 0, 10000, 2, &okM1);
        if(okM2){
            newStepsPerMm=(calibrationDistance/qAbs((secondMeasurment-firstMeasurment)))*currentStepsPerMm;
            ui->yCurrentSteps->setText(QString::number(newStepsPerMm));
            emit writeToPrinter("M92 Y"+ui->yCurrentSteps->text());
            double error =qAbs((double)1-(calibrationDistance/(secondMeasurment-firstMeasurment)));
            if(error>0.01){

                QMessageBox *msgBox = new QMessageBox(this);
                msgBox->setWindowTitle(tr("Error"));
                msgBox->setText(tr("Error is %1 %, continue callibrating?").arg(QString::number(error*100)));

                QPushButton *yesButton = msgBox->addButton(tr("Yes"), QMessageBox::ActionRole);
                msgBox->addButton(tr("No"), QMessageBox::ActionRole);
                msgBox->exec();

                //If user clicks 'Yes'  button , accept QCloseEvent (Close Window)
                if ((QPushButton*)msgBox->clickedButton() == yesButton)
                {
                   on_calibrateYBtn_clicked();
                }
            }
        }
    }
}

void CalibrateDialog::on_calibrateZBtn_clicked()
{
    bool okM1=false;
    bool okM2=false;
    emit writeToPrinter("G28 Z0");
    emit writeToPrinter("M92 Z"+ui->zCurrentSteps->text());
    double currentStepsPerMm = ui->zCurrentSteps->text().toDouble();
    double newStepsPerMm = ui->zCurrentSteps->text().toDouble();
    double calibrationDistance = 50;
    double firstMeasurment = QInputDialog::getDouble(this, tr("Input first measurment"),
                                       tr("Measurment"), 0, 0, 10000, 2, &okM1);
    if (okM1){
        emit writeToPrinter("G1 Z"+QString::number(calibrationDistance)+" F200");
        double secondMeasurment = QInputDialog::getDouble(this, tr("Input second measurment"),
                                           tr("Measurment"), calibrationDistance, 0, 10000, 2, &okM1);
        if(okM2){
            newStepsPerMm=(calibrationDistance/qAbs((secondMeasurment-firstMeasurment)))*currentStepsPerMm;
            ui->zCurrentSteps->setText(QString::number(newStepsPerMm));
            emit writeToPrinter("M92 Z"+ui->zCurrentSteps->text());
            double error =qAbs((double)1-(calibrationDistance/(secondMeasurment-firstMeasurment)));
            if(error>0.01){

                QMessageBox *msgBox = new QMessageBox(this);
                msgBox->setWindowTitle(tr("Error"));
                msgBox->setText(tr("Error is %1 %, continue callibrating?").arg(QString::number(error*100)));

                QPushButton *yesButton = msgBox->addButton(tr("Yes"), QMessageBox::ActionRole);
                msgBox->addButton(tr("No"), QMessageBox::ActionRole);
                msgBox->exec();

                //If user clicks 'Yes'  button , accept QCloseEvent (Close Window)
                if ((QPushButton*)msgBox->clickedButton() == yesButton)
                {
                   on_calibrateZBtn_clicked();
                }
            }
        }
    }
}
