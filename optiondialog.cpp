#include "optiondialog.h"
#include "ui_optiondialog.h"

OptionDialog::OptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionDialog)
{
    ui->setupUi(this);
    this->materials = new QList<Material*>;

}
OptionDialog::~OptionDialog()
{
    delete ui;
}

void OptionDialog::on_closeBtn_clicked()
{
    emit newSize(QVector3D(ui->sizeX->value(),ui->sizeY->value(),ui->sizeZ->value()));
    this->hide();
}

void OptionDialog::setSlicerDir(QString dir){
    ui->slicerDir->setText(dir);
}

QString OptionDialog::getSlicerDir(){
    return ui->slicerDir->text();
}

void OptionDialog::setConfigDir(QString dir){
    ui->configDir->setText(dir);
}

QString OptionDialog::getConfigDir(){
    return ui->configDir->text();
}

void OptionDialog::setOutputDir(QString dir){
    ui->outputDir->setText(dir);
}

QString OptionDialog::getOutputDir(){
    return ui->outputDir->text();
}

QVector3D OptionDialog::getSize(){
    return QVector3D(ui->sizeX->value(),ui->sizeY->value(),ui->sizeZ->value());
}

void OptionDialog::setSize(QVector3D size){
    ui->sizeX->setValue(size.x());
    ui->sizeY->setValue(size.y());
    ui->sizeZ->setValue(size.z());
}

void OptionDialog::on_slicerDirBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Find slicer exe"), ui->slicerDir->text(), tr("Slic3r-console (slic3r-console.exe)"));
    ui->slicerDir->setText(fileName);
    emit slicerPathChanged(fileName);
}

void OptionDialog::on_configDirBtn_clicked()
{

    QString dirName = QFileDialog::getExistingDirectory(this,"Choose configs dir", ui->slicerDir->text());
    ui->configDir->setText(dirName);
}

void OptionDialog::on_outputDirBtn_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this,"Choose output dir", ui->slicerDir->text());
    ui->outputDir->setText(dirName);
    emit outputPathChanged(dirName);
}

void OptionDialog::on_extruderNum_valueChanged(int arg1)
{
    if(arg1>ui->tableWidget->rowCount()){
        addExtruder(qMakePair(arg1-1,QString("#ff0000")));
    }
    else{
        ui->tableWidget->setRowCount(arg1);
        this->materials->takeLast();
        emit newList(this->materials);
    }
}

QList< QPair<int, QString > > OptionDialog::getExtruders(){
    QList< QPair<int, QString > > out;
    QLabel* label;
    for(int i=0; i<ui->tableWidget->rowCount(); i++){
        label  = (QLabel*)ui->tableWidget->cellWidget(i,1);
        out.append(qMakePair(i,label->text()));
    }
     return out;
}

void OptionDialog::addExtruder(QPair<int, QString> extruder){
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
    QLabel* label = new QLabel();
    QLabel* label2 = new QLabel();
    label2->setText("T"+QString::number(extruder.first));
    QColor color(extruder.second);
    label->setStyleSheet("background-color: rgb("+QString::number(color.red())+","+QString::number(color.green())+","+QString::number(color.blue())+");");
    label->setText(extruder.second);
    ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1,0,label2);
    ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1,1,label);
    ui->extruderNum->blockSignals(true);
    ui->extruderNum->setValue(ui->tableWidget->rowCount());
    ui->extruderNum->blockSignals(false);
    emit newList(this->materials);
    this->materials->append(new Material(color,extruder.first));
}


void OptionDialog::on_tableWidget_cellDoubleClicked(int row, int column)
{
    qDebug() << row << column;
    if(column==1){
        QLabel* label = (QLabel*)ui->tableWidget->cellWidget(row,column);
        QColor color=QColorDialog::getColor(QColor(label->text()),this);
        qDebug() << color;
        label->setText(color.name());
        label->setStyleSheet("background-color: rgb("+QString::number(color.red())+","+QString::number(color.green())+","+QString::number(color.blue())+");");
        this->materials->at(row)->changeColor(color);
    }
}
