#include "slicedialog.h"
#include "ui_slicedialog.h"

SliceDialog::SliceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SliceDialog)
{
    ui->setupUi(this);
    this->stlView = new StlView(ui->stlViewWidget);
    ui->stlViewWidget->layout()->addWidget(this->stlView);
    connect(this->stlView, SIGNAL(objectPicked(bool)), ui->objectOptionGroup, SLOT(setEnabled(bool)));
    connect(this->stlView, SIGNAL(objectPicked(bool)), ui->removeBtn, SLOT(setEnabled(bool)));
    connect(this->stlView, SIGNAL(selectedRotation(int)), this, SLOT(objectRotated(int)));
    connect(this->stlView, SIGNAL(selectedScale(int)), this, SLOT(objectScaled(int)));
    connect(this->stlView, SIGNAL(progress(int,int,QString)), this, SLOT(updateProgress(int,int,QString)));
    connect(this->stlView, SIGNAL(doneProcessing(bool)),ui->progressBar,SLOT(setHidden(bool)));

    connect(this->stlView, SIGNAL(selectedCors(QPointF)), this, SLOT(setOffset(QPointF)));

    connect(ui->rotationsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(rotateObject(int)));
    connect(ui->scaleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(scaleObject(int)));
    connect(ui->xSpinBox, SIGNAL(valueChanged(double)), this, SLOT(moveObjectX(double)));
    connect(ui->ySpinBox, SIGNAL(valueChanged(double)), this, SLOT(moveObjectY(double)));
    connect(this->stlView, SIGNAL(nonManifold(QString)), this, SLOT(nonManifold(QString)));
    ui->consoleGroup->hide();
    ui->objectOptionGroup->setEnabled(false);
    ui->removeBtn->setEnabled(false);
    ui->progressBar->hide();

    QMovie *movie = new QMovie(":/imgs/loader.gif");
    ui->loaderLabel->setMovie(movie);
    movie->start();
}

void SliceDialog::setOffset(QPointF point){
    ui->xSpinBox->blockSignals(true);
    ui->xSpinBox->setValue(point.x()*10);
    ui->xSpinBox->blockSignals(false);
    ui->ySpinBox->blockSignals(true);
    ui->ySpinBox->setValue(point.y()*10);
    ui->ySpinBox->blockSignals(false);
}

void SliceDialog::moveObjectX(double x){
    this->stlView->moveObject(QPointF(x/10,ui->ySpinBox->value()/10));
}

void SliceDialog::moveObjectY(double y){
   this->stlView->moveObject(QPointF(ui->xSpinBox->value()/10,y/10));
}

void SliceDialog::rotateObject(int ang){
    this->stlView->rotateObject((double)(360-ang));
}

void SliceDialog::objectRotated(int ang){
    ui->rotationsSpinBox->blockSignals(true);
    ui->rotationsSpinBox->setValue(ang);
    ui->rotationsSpinBox->blockSignals(false);
}

void SliceDialog::objectScaled(int scale){
    ui->scaleSpinBox->blockSignals(true);
    ui->scaleSpinBox->setValue(scale);
    ui->scaleSpinBox->blockSignals(false);
}

void SliceDialog::scaleObject(int scale){
    this->stlView->scaleObject((double)scale/100);
}

void SliceDialog::nonManifold(QString name){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("STL isnt manifold!"));
    msgBox.exec();

}

SliceDialog::~SliceDialog()
{
    delete ui;
}

void SliceDialog::updateConfigs(QString dir){
    QDir confDir(dir);
    this->configPath = dir;
    ui->confCombo->clear();
    ui->confCombo->addItems(confDir.entryList(QStringList("*.ini")));
}

void SliceDialog::setLastDir(QString dir){
    this->lastDir=dir;
}

void SliceDialog::clearObjects(){
    this->stlView->removeObject();
}

void SliceDialog::addObject(QString file){
    this->stlView->addObject(file);
}

void SliceDialog::on_addBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Find slicer exe"), this->lastDir, tr("STL files (*.stl)"));
    if(fileName!=""){
        this->addObject(fileName);
        this->lastDir = fileName.left(fileName.lastIndexOf("/"));
    }
}

void SliceDialog::updateSlicerPath(QString path){
    this->slicerPath=path;
}

void SliceDialog::updateOutputPath(QString path){
    this->outputPath=path;
}

void SliceDialog::on_sliceBtn_clicked()
{
    QString stlFile = saveStl("./temp.stl");
    QStringList arguments;
    QString fileName = stlView->getObject(stlView->getColorsList().at(0))->getFileName();
    fileName.remove(0,fileName.lastIndexOf("/")+1);
    fileName=fileName.left(fileName.lastIndexOf("."));
    this->outputFile = QFileDialog::getSaveFileName(this, tr("Save stl"), this->lastDir+"/"+fileName+".gcode", tr("Gcode files (*.gcode)"));
    if(QFile::exists(this->outputFile)){
        QFile::remove(this->outputFile);
    }
    if(this->outputFile!=""){
        arguments.append(stlFile);
        arguments.append("--load");
        arguments.append(this->configPath+"/"+ui->confCombo->currentText());
        arguments.append("--output");
        arguments.append(this->outputFile);
        arguments.append("--print-center");
        arguments.append(QString::number((int)this->printCenter.x())+","+QString::number((int)this->printCenter.y()));
        arguments.append("--layer-height");
        arguments.append(QString::number(ui->layerHeight->value()));
        arguments.append("--fill-density");
        arguments.append(QString::number((double)ui->fillDensity->value()/100));
        qDebug() << arguments;
        this->slicerProcess = new QProcess(this);
        connect(this->slicerProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(updateStatus()));
        connect(this->slicerProcess,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slicingFinished(int,QProcess::ExitStatus)));
        slicerProcess->start(this->slicerPath,arguments);
        ui->consoleGroup->show();
    }
}

void SliceDialog::slicingFinished(int exitCode, QProcess::ExitStatus exitStatus){
    qDebug() << "ok" << exitCode;
    delete this->slicerProcess;
    emit fileSliced(this->outputFile);
    ui->consoleGroup->hide();
    this->hide();
    QFile::remove("./temp.stl");
}

void SliceDialog::updateStatus(){
    //qDebug() << this->slicerProcess->readAllStandardOutput().trimmed();
    ui->console->appendPlainText(this->slicerProcess->readAllStandardOutput().trimmed());
}

void SliceDialog::setTableSize(int x, int y){
    this->stlView->setTableSize(x,y);
    this->xSize=x;
    this->ySize=y;
}

void SliceDialog::on_removeBtn_clicked()
{
    this->stlView->removeObject();
}

QString SliceDialog::saveStl(QString fileName)
{
    if(fileName==""){
        fileName = QFileDialog::getSaveFileName(this, tr("Save stl"), this->lastDir, tr("STL files (*.stl)"));
        this->lastDir = fileName.left(fileName.lastIndexOf("/"));
    }

    QList<QVector3D> data;
    QStringList objects=stlView->getColorsList();
    for(int i=0; i<objects.size(); i++){
        data.append(this->stlView->getObject(objects.at(i))->getTriangles());
    }
    int trianglesSize=data.size()/4;

    if(fileName!=""){
        //for finding print center
        qreal xMax=0,xMin=0;
        qreal yMax=0,yMin=0;
        QFile* file = new QFile(fileName);
        if(file->open(QIODevice::WriteOnly)){
            QDataStream out(file);
            out.setByteOrder(QDataStream::LittleEndian);
            out.setFloatingPointPrecision(QDataStream::SinglePrecision);
            for (int i = 0; i < 20; i++) out << (int)0;
            out << trianglesSize;
            for(int i=0; i<data.size(); i+=4){
                emit updateProgress(i,data.size(),tr("Saving stl"));
                out << (float)data.at(i).x();
                out << (float)data.at(i).y();
                out << (float)data.at(i).z();
                out << (float)data.at(i+1).x();
                out << (float)data.at(i+1).y();
                out << (float)data.at(i+1).z();
                out << (float)data.at(i+2).x();
                out << (float)data.at(i+2).y();
                out << (float)data.at(i+2).z();
                out << (float)data.at(i+3).x();
                out << (float)data.at(i+3).y();
                out << (float)data.at(i+3).z();
                out << (short)0;
                if(i==0){
                    xMax=data.at(i+1).x();
                    yMax=data.at(i+1).y();
                    xMin=data.at(i+1).x();
                    yMin=data.at(i+1).y();
                }
                xMax=qMax(qMax(xMax,data.at(i+1).x()),qMax(data.at(i+2).x(),data.at(i+3).x()));
                yMax=qMax(qMax(yMax,data.at(i+1).y()),qMax(data.at(i+2).y(),data.at(i+3).y()));
                xMin=qMin(qMin(xMin,data.at(i+1).x()),qMin(data.at(i+2).x(),data.at(i+3).x()));
                yMin=qMin(qMin(yMin,data.at(i+1).y()),qMin(data.at(i+2).y(),data.at(i+3).y()));
            }

        }
        QRectF rect(QPointF(xMin,yMax),QPointF(xMax,yMin));
        this->printCenter=rect.center().toPoint();
        qDebug() << xMax << xMin << yMax << yMin;
        file->close();
    }
    ui->progressBar->hide();
    return fileName;
}

void SliceDialog::on_exportStlBtn_clicked()
{
    this->saveStl("");
}

void SliceDialog::on_confCombo_currentIndexChanged(const QString &arg1)
{
    QSettings settings(this->configPath+"/"+arg1, QSettings::IniFormat);
    ui->fillDensity->setValue((int)(settings.value("fill_density").toDouble()*100));
    ui->layerHeight->setValue(settings.value("layer_height").toDouble());
}

void SliceDialog::on_mirrorXBtn_clicked()
{
    this->stlView->mirrorObject('x');
}

void SliceDialog::on_mirrorYBtn_clicked()
{
    this->stlView->mirrorObject('y');
}

void SliceDialog::on_mirrorZBtn_clicked()
{
   this->stlView->mirrorObject('z');
}

void SliceDialog::on_repeairNormals_clicked()
{
     this->stlView->repeairObjectNormals();
}

void SliceDialog::updateProgress(int value, int max, QString text){
    if(ui->progressBar->isHidden()){
        ui->progressBar->show();
    }

    ui->progressBar->setMaximum(max);
    ui->progressBar->setValue(value);
    ui->progressBar->setFormat(text);
    qApp->processEvents();
}

void SliceDialog::on_repairHoles_clicked()
{
   this->stlView->repeairObjectHoles();
}

void SliceDialog::on_duplicateBtn_clicked()
{
    this->stlView->duplicateObject();
}

void SliceDialog::on_mToCenterBtn_clicked()
{
    //this->stlView->getObject(this->selectedObject)->moveXY(((double)this->xSize/2)*0.1,((double)this->ySize/2)*0.1);
    this->stlView->update();
}
