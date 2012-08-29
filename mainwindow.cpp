#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //about window
    this->aboutWindow = new AboutWindow();
    this->aboutWindow->hide();
    this->aboutWindow->move(this->geometry().center()-this->aboutWindow->geometry().center());
    //calibrate Dialog
    this->calibrateDialog = new CalibrateDialog();
    this->calibrateDialog->hide();
    this->calibrateDialog->move(this->geometry().center()-this->calibrateDialog->geometry().center());
    //option Dialog
    this->optionDialog = new OptionDialog();
    this->optionDialog->hide();
    this->optionDialog->move(this->geometry().center()-this->optionDialog->geometry().center());
    //slice dialog
    this->sliceDialog = new SliceDialog(ui->glWidget, this);
    this->sliceDialog->hide();
    this->sliceDialog->move(this->geometry().center()-this->sliceDialog->geometry().center());
    connect(this->optionDialog, SIGNAL(slicerPathChanged(QString)), this->sliceDialog, SLOT(updateSlicerPath(QString)));
    connect(this->optionDialog, SIGNAL(outputPathChanged(QString)), this->sliceDialog, SLOT(updateOutputPath(QString)));
    connect(this->optionDialog, SIGNAL(newSize(QVector3D)), this, SLOT(updatadeSize(QVector3D)));
    connect(this->optionDialog, SIGNAL(newList(QList<Material*>*)), this->sliceDialog, SLOT(setMaterialList(QList<Material*>*)));
    connect(this->sliceDialog, SIGNAL(fileSliced(QString)), this, SLOT(loadFile(QString)));
    //set version number
    this->setWindowTitle("YARRH v"+QString::number(VERSION_MAJOR)+"."+QString::number(VERSION_MINOR)+"."+QString::number(VERSION_REVISION));
    this->aboutWindow->setVersion(VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
    //setting up printer and its thread
    this->printerObj = new Printer();
    QThread *qthread = new QThread();

    //connecting ui to printer
    connect(printerObj, SIGNAL(write_to_console(QString)), ui->inConsole, SLOT(appendPlainText(QString)), Qt::QueuedConnection);
    connect(ui->fanSpinBox, SIGNAL(valueChanged(int)), printerObj, SLOT(setFan(int)), Qt::QueuedConnection);
    ui->fanSpinBox->blockSignals(true);
    //connecting move btns
    connect(ui->homeX, SIGNAL(clicked()), printerObj, SLOT(homeX()), Qt::QueuedConnection);
    connect(ui->homeY, SIGNAL(clicked()), printerObj, SLOT(homeY()), Qt::QueuedConnection);
    connect(ui->homeAll, SIGNAL(clicked()), printerObj, SLOT(homeAll()), Qt::QueuedConnection);
    //connect monit temp checkbox
    connect(ui->graphGroupBox, SIGNAL(toggled(bool)), printerObj, SLOT(setMonitorTemperature(bool)),Qt::QueuedConnection);
    //connect printer to temp widget
    connect(printerObj, SIGNAL(currentTemp(double,double,double)), this, SLOT(drawTemp(double,double,double)));
    connect(printerObj, SIGNAL(progress(int)), this, SLOT(updateProgress(int)));
    connect(printerObj, SIGNAL(connected(bool)), this, SLOT(printerConnected(bool)));
    //setting ui temp from gcode
    connect(printerObj, SIGNAL(settingTemp1(double)), this, SLOT(setTemp1FromGcode(double)));
    connect(printerObj, SIGNAL(settingTemp3(double)), this, SLOT(setTemp3FromGcode(double)));
    //updating head position in ui
    connect(printerObj, SIGNAL(currentPosition(QVector3D)), this, SLOT(updateHeadPosition(QVector3D)));
    //print finished signal
    connect(printerObj, SIGNAL(printFinished(bool)), this, SLOT(printFinished(bool)));
    //connect calibration dialog to printer
    connect(calibrateDialog, SIGNAL(writeToPrinter(QString)), printerObj, SLOT(send_now(QString)),Qt::QueuedConnection);
    //connect z slider
    connect(ui->zSlider, SIGNAL(valueChanged(int)), this, SLOT(moveZ(int)));
    connect(ui->zSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateZ(int)));
    //connect action load
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(loadFile()));

    printerObj->moveToThread(qthread);
    qthread->start(QThread::HighestPriority);


    this->portEnum = new QextSerialEnumerator(this);
    this->portEnum->setUpNotifications();
    QList<QextPortInfo> ports = this->portEnum->getPorts();
    //finding avalible ports
    foreach (QextPortInfo info, ports) {
        ui->portCombo->addItem(info.portName);
    }

    //connecting travel moves checkbox
    connect(ui->showTravelChkBox, SIGNAL(toggled(bool)),ui->glWidget, SLOT(showTravel(bool)));
    //disable steppers btn
    connect(ui->disableStpBtn, SIGNAL(clicked()), printerObj, SLOT(disableSteppers()), Qt::QueuedConnection);
    //connect head move widget
    connect(ui->headControlWidget, SIGNAL(clicked(QPoint)), this, SLOT(moveHead(QPoint)));
    connect(ui->headControlWidget, SIGNAL(hovered(QPoint)), this, SLOT(updateHeadGoToXY(QPoint)));

    ui->printBtn->setDisabled(true);
    ui->pauseBtn->setDisabled(true);
    ui->axisControlGroup->setDisabled(true);
    ui->headControlWidget->hidePoints(true);
    ui->temperatureGroupBox->setDisabled(true);
    this->calibrateDialog->setDisabled(true);
    ui->progressBar->hide();
    ui->baudCombo->addItem("1200", BAUD1200);
    ui->baudCombo->addItem("2400", BAUD2400);
    ui->baudCombo->addItem("4800", BAUD4800);
    ui->baudCombo->addItem("9600", BAUD9600);
    ui->baudCombo->addItem("19200", BAUD19200);
    ui->baudCombo->addItem("38400", BAUD38400);
    ui->baudCombo->addItem("57600", BAUD57600);
    ui->baudCombo->addItem("115200", BAUD115200);
#if defined(Q_OS_WIN) || defined(qdoc)
    ui->baudCombo->addItem("250000", BAUD250000);
#endif  //Q_OS_WIN
    ui->baudCombo->setCurrentIndex(8);
    //restore user settings
    restoreSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}


//connecting to port
void MainWindow::on_connectBtn_toggled(bool connect)
{
    //connecting to printer port
    if(connect){
        ui->connectBtn->setText("Connecting");
        QMetaObject::invokeMethod(printerObj,"connectPort",Qt::QueuedConnection,Q_ARG(QString, ui->portCombo->currentText()),Q_ARG(int,ui->baudCombo->itemData(ui->baudCombo->currentIndex()).toInt()));
    }
    else{
        QMetaObject::invokeMethod(printerObj,"stopPrint",Qt::QueuedConnection);
        QMetaObject::invokeMethod(printerObj,"disconnectPort",Qt::QueuedConnection);
    }
}

void MainWindow::printerConnected(bool connected){
    //connecting to printer port
    if(connected){
            ui->connectBtn->setText(tr("Disconnect"));
            ui->connectBtn->setIcon(QIcon(":/imgs/icons/disconnect.png"));
            ui->axisControlGroup->setDisabled(false);
            ui->headControlWidget->hidePoints(false);
            ui->temperatureGroupBox->setDisabled(false);
            this->calibrateDialog->setDisabled(false);
            if(this->gcodeLines.size()>0){
                if(ui->progressBar->value()>0){
                    ui->pauseBtn->setEnabled(true);
                }
                if(ui->pauseBtn->text()!=tr("Resume")){
                    ui->printBtn->setEnabled(true);
                }
            }
            if(this->calibrateDialog->autoCalibrateX()){
                this->printerObj->writeToPort("M92 X"+QString::number(this->calibrateDialog->getCallibrationsSetting().x()),false);
            }
            if(this->calibrateDialog->autoCalibrateY()){
                this->printerObj->writeToPort("M92 Y"+QString::number(this->calibrateDialog->getCallibrationsSetting().y()),false);
            }
            if(this->calibrateDialog->autoCalibrateZ()){
                this->printerObj->writeToPort("M92 Z"+QString::number(this->calibrateDialog->getCallibrationsSetting().z()),false);
            }
    }
    else{
        ui->connectBtn->setText(tr("Connect"));
        ui->connectBtn->setIcon(QIcon(":/imgs/icons/connect.png"));
        ui->connectBtn->blockSignals(true);
        ui->connectBtn->setChecked(false);
        ui->connectBtn->blockSignals(false);

        ui->axisControlGroup->setDisabled(true);
        ui->headControlWidget->hidePoints(true);
        ui->temperatureGroupBox->setDisabled(true);
        this->calibrateDialog->setDisabled(true);

        ui->printBtn->setEnabled(false);
        ui->pauseBtn->setEnabled(false);
    }
}

//loading file
void MainWindow::loadFile(QString fileName){
    if(fileName==""){
        fileName = QFileDialog::getOpenFileName(this, tr("Open file"), this->lastOpendDir, tr("Print files (*.g *.gcode *.stl)"));
        this->lastOpendDir=fileName.left(fileName.lastIndexOf("/"));
        this->sliceDialog->setLastDir(this->lastOpendDir);
    }
    // if its stl then open slice window
    if(fileName.endsWith(".stl",Qt::CaseInsensitive)){
        this->sliceDialog->move(QPoint(this->geometry().center().x()-this->sliceDialog->width()/2,this->geometry().center().y()-this->sliceDialog->height()/2));
        this->sliceDialog->show();
        this->sliceDialog->updateStlView();
        this->sliceDialog->updateConfigs(this->optionDialog->getConfigDir());
        //this->sliceDialog->clearObjects();


        this->sliceDialog->addObject(fileName);
    }
    //else just load gcode
    else{
        //show filename in ui
        ui->groupBox_4->setTitle(tr("File")+": "+fileName.right(fileName.length()-fileName.lastIndexOf("/")-1));
        //open file
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        //set last opend dir
        this->lastOpendDir=fileName.left(fileName.lastIndexOf("/"));

        //clear last object gcode
        this->gcodeLines.clear();
        //clear gl widget
        ui->glWidget->clearObjects();
        //show progress dialog
        ui->progressBar->show();
        ui->progressBar->setFormat(tr("Parsing file"));

        //buffer filecontent
        this->fileContent.clear();
        this->fileContent=file.readAll();
        file.close();

        //split buffer to lines
        QStringList gcodesTemp=this->fileContent.split("\n");
        QString temp;
        //set max value of progress dialog
        ui->progressBar->setMaximum(gcodesTemp.size());

        //parsing input file
        qreal x=0,y=0,z=-1, travel=0;
        GCodeObject* tempObject = new GCodeObject(ui->glWidget);
        int layerCount=0;
        float prevZ=0;
        for(int i=0; i<gcodesTemp.size(); i++){
            ui->progressBar->setValue(i);
            //refresh ui
            qApp->processEvents();
            temp=gcodesTemp.at(i);
            temp.replace("/n","");
            if(temp.contains("filament used")){
                ui->filamentLbl->setText(temp.right(temp.length()-temp.lastIndexOf("=")-2));
            }
            temp=temp.left(temp.lastIndexOf(";"));
            temp=temp.trimmed();
            if(temp!=""){
                this->gcodeLines.append(temp);
            }

            //parsing lines to get point coordinates
            if(temp.contains("X")){
                x=(qreal)temp.mid(temp.indexOf("X")+1,temp.indexOf(" ",temp.indexOf("X"))-temp.indexOf("X")).toFloat();
            }
            if(temp.contains("Y")){
                y=(qreal)temp.mid(temp.indexOf("Y")+1,temp.indexOf(" ",temp.indexOf("Y"))-temp.indexOf("Y")).toFloat();
            }
            if(temp.contains("Z")){
                z=(qreal)temp.mid(temp.indexOf("Z")+1,temp.indexOf(" ",temp.indexOf("Z"))-temp.indexOf("Z")).toFloat();
                if(z>prevZ){
                    layerCount++;
                }
                prevZ=z;
            }
            if(temp.contains("X") || temp.contains("Y") || temp.contains("Z")){
                if(temp.contains("E") || temp.contains("A")){
                    travel=0;
                }
                else{
                    travel=1;
                }
                tempObject->addVertex(x,y,z,travel,layerCount);
            }
        }
        ui->layerScrollBar->setMaximum(layerCount+1);
        ui->layerScrollBar->setValue(1);
        ui->currentLayer->setText(QString::number(layerCount)+"/"+QString::number(layerCount));
        ui->progressBar->hide();

        ui->progressBar->setMaximum(this->gcodeLines.size());
        ui->progressBar->setValue(0);
        ui->glWidget->setLayers(ui->layerScrollBar->maximum());
        tempObject->render(0.01);
        ui->glWidget->addObject(tempObject);

        //enable print button
        if(printerObj->isConnected()){
            ui->printBtn->setEnabled(true);
        }
        //disable pause btn
        ui->pauseBtn->setEnabled(false);
        ui->pauseBtn->blockSignals(true);
        ui->pauseBtn->setChecked(false);
        ui->pauseBtn->blockSignals(false);
        ui->progressBar->hide();
        ui->pauseBtn->setText(tr("Pause"));
        ui->printBtn->setText(tr("Print"));
        ui->printBtn->setIcon(QIcon(":/imgs/icons/print.png"));
        ui->pauseBtn->setIcon(QIcon(":/imgs/icons/pause.png"));
        this->currentLayer=0;
        this->lastZ=0;
        ui->glWidget->setCurrentLayer(0);
        ui->headControlWidget->resetLayer();
    }
}

//printing object
void MainWindow::on_printBtn_clicked(){
    if(printerObj->isConnected()){
        this->startTime=QTime::currentTime();
        ui->inConsole->appendPlainText(tr(QString("Print started at "+ this->startTime.toString("hh:mm:ss")).toAscii()));
        QMetaObject::invokeMethod(printerObj,"loadToBuffer",Qt::DirectConnection,Q_ARG(QStringList, this->gcodeLines), Q_ARG(bool, true));
        ui->progressBar->setMaximum(this->gcodeLines.size());
        QMetaObject::invokeMethod(printerObj,"startPrint",Qt::QueuedConnection);
        ui->pauseBtn->setEnabled(true);
        //disable pause btn
        ui->pauseBtn->setEnabled(true);
        ui->pauseBtn->blockSignals(true);
        ui->pauseBtn->setChecked(false);

        ui->pauseBtn->blockSignals(false);
        ui->pauseBtn->setText(tr("Pause"));
        ui->pauseBtn->setIcon(QIcon(":/imgs/icons/pause.png"));
        ui->printBtn->setEnabled(false);
        ui->progressBar->show();
        this->calibrateDialog->setDisabled(true);

        //disable axis control while printing
        ui->axisControlGroup->setDisabled(true);
        ui->headControlWidget->hidePoints(true);
        this->lastZ=0;
        this->currentLayer=0;
        ui->glWidget->setCurrentLayer(this->currentLayer);
        ui->headControlWidget->resetLayer();
        ui->printBtn->setText(tr("Restart"));
        ui->printBtn->setIcon(QIcon(":/imgs/icons/reload.png"));
    }
}

//setting layers displayed
void MainWindow::on_layerScrollBar_valueChanged(int layers){
    ui->glWidget->setLayers(ui->layerScrollBar->maximum()-layers+1);
    ui->currentLayer->setText(QString::number(ui->layerScrollBar->maximum()-layers)+"/"+QString::number(ui->layerScrollBar->maximum()-1));
}

//slot to connect diffrent signals
void MainWindow::moveHead(QPoint point){
    QMetaObject::invokeMethod(printerObj,"moveHead",Qt::QueuedConnection,Q_ARG(QPoint, QPoint(point.x(),this->optionDialog->getSize().y()*10-point.y())),Q_ARG(int, ui->speedSpinBox->value()*60));
    ui->yAt->setText("Y: "+QString::number(this->optionDialog->getSize().y()*10-point.y()));
    ui->xAt->setText("X: "+QString::number(point.x()));
}

//pausing print

void MainWindow::on_pauseBtn_toggled(bool pause){
    if(pause){
        ui->pauseBtn->setText(tr("Resume"));
        ui->pauseBtn->setIcon(QIcon(":/imgs/icons/resume.png"));
        QMetaObject::invokeMethod(printerObj,"pausePrint", Qt::DirectConnection,Q_ARG(bool, pause));
    }
    else{
        ui->pauseBtn->setText(tr("Pause"));
        ui->pauseBtn->setIcon(QIcon(":/imgs/icons/pause.png"));
        QMetaObject::invokeMethod(printerObj,"pausePrint", Qt::DirectConnection,Q_ARG(bool, pause));
    }
    ui->axisControlGroup->setDisabled(!pause);
    ui->headControlWidget->hidePoints(!pause);
    this->calibrateDialog->setDisabled(!pause);
    ui->printBtn->setEnabled(pause);
}

//draw temp on graph

void MainWindow::drawTemp(double t1, double t2, double hb){
    ui->tempGraphWidget->addMeasurment(t1,t2,hb,QDateTime::currentMSecsSinceEpoch()/1000);
    ui->t1Label->setText(QString::number(t1)+" Â°C");
    ui->t3Label->setText(QString::number(hb)+" Â°C");
}

//update print progress
void MainWindow::updateProgress(int progress){
    ui->progressBar->setValue(ui->progressBar->maximum()-progress);
    //calculating ETA
    this->durationTime=QTime(0,0,0);

    this->durationTime=this->durationTime.addSecs(startTime.secsTo(QTime::currentTime()));

    float linesPerSec=(float)ui->progressBar->value()/(float)startTime.secsTo(QTime::currentTime());
    this->eta=QTime(0,0,0).addSecs((int)((float)progress/linesPerSec));
    this->eta=this->eta.addSecs(startTime.secsTo(QTime::currentTime()));

    ui->progressBar->setFormat(tr("Layer %1/%2 ").arg(this->currentLayer).arg(ui->layerScrollBar->maximum()-1)+this->durationTime.toString("hh:mm:ss")+"/"+this->eta.toString("hh:mm:ss")+" %p%");
}


void MainWindow::on_t1Btn_toggled(bool on){
    bool ok;
    int value = ui->t1Combo->currentText().toInt(&ok)%300;
    if(ok){
        if(on){
            ui->t1Btn->setText("Off");
            ui->tempGraphWidget->setTargets(value,-1,-1);
            QMetaObject::invokeMethod(printerObj,"setTemp1",Qt::QueuedConnection,Q_ARG(int, value));
            QStringList tempList;
            for(int i=0; i<ui->t1Combo->count(); i++){
                tempList.append(ui->t1Combo->itemText(i));
            }
            if(!tempList.contains(ui->t1Combo->currentText())){
                ui->t1Combo->addItem(ui->t1Combo->currentText());
            }
        }
        else{
            ui->t1Btn->setText("On");
            ui->tempGraphWidget->setTargets(0,-1,-1);
            QMetaObject::invokeMethod(printerObj,"setTemp1",Qt::QueuedConnection,Q_ARG(int, 0));
        }
    }
}

void MainWindow::setTemp2(bool on){

}

void MainWindow::on_hbBtn_toggled(bool on){
    bool ok;
    int value = ui->hbCombo->currentText().toInt(&ok)%300;
    if(ok){
        if(on){
            ui->hbBtn->setText("Off");
            ui->tempGraphWidget->setTargets(-1,-1,value);
            QMetaObject::invokeMethod(printerObj,"setTemp3",Qt::QueuedConnection,Q_ARG(int, value));

            QStringList tempList;
            for(int i=0; i<ui->hbCombo->count(); i++){
                tempList.append(ui->hbCombo->itemText(i));
            }
            if(!tempList.contains(ui->hbCombo->currentText())){
                ui->hbCombo->addItem(ui->hbCombo->currentText());
            }
        }
        else{
            ui->hbBtn->setText("On");
            ui->tempGraphWidget->setTargets(-1,-1,0);
            QMetaObject::invokeMethod(printerObj,"setTemp3",Qt::QueuedConnection,Q_ARG(int, 0));
        }
    }
}

void MainWindow::setTemp1FromGcode(double value){
    ui->tempGraphWidget->setTargets(value,-1,-1);
    if(value>0){
        ui->t1Btn->blockSignals(true);
        ui->t1Btn->setText("Off");
        ui->t1Btn->setChecked(true);
        ui->t1Btn->blockSignals(false);
    }
    else{
        ui->t1Btn->blockSignals(true);
        ui->t1Btn->setText("On");
        ui->t1Btn->setChecked(false);
        ui->t1Btn->blockSignals(false);
    }
}

void MainWindow::setTemp3FromGcode(double value){
    ui->tempGraphWidget->setTargets(-1,-1,value);
    if(value>0){
        ui->hbBtn->blockSignals(true);
        ui->hbBtn->setText("Off");
        ui->hbBtn->setChecked(true);
        ui->hbBtn->blockSignals(false);
    }
    else{
        ui->hbBtn->blockSignals(true);
        ui->hbBtn->setText("On");
        ui->hbBtn->setChecked(false);
        ui->hbBtn->blockSignals(false);
    }
}
void MainWindow::moveZ(int value){
    ui->zMoveTo->setText("Z: "+QString::number(0));
    QMetaObject::invokeMethod(printerObj,"moveHeadZ",Qt::QueuedConnection,Q_ARG(double, (double)value),Q_ARG(int, ui->speedZSpinBox->value()*60), Q_ARG(bool, false));
}

void MainWindow::updateZ(int value){
    ui->zMoveTo->setText("Z: "+QString::number(value));
}

//updates head position and current layer num
void MainWindow::updateHeadPosition(QVector3D point){
    ui->xAt->setText("X: "+QString::number(point.x()));
    ui->yAt->setText("Y: "+QString::number(point.y()));
    ui->zAt->setText("Z: "+QString::number(point.z()));
    if(point.z()>this->lastZ && this->printerObj->getIsPrinting()){
        this->lastZ=point.z();
        this->currentLayer++;
        ui->glWidget->setCurrentLayer(this->currentLayer);
        ui->headControlWidget->resetLayer();
    }
    ui->headControlWidget->addPrintedPoint(point.toPointF());
}

void MainWindow::updateHeadGoToXY(QPoint point){
    ui->xMoveTo->setText("X: "+QString::number(point.x()));
    ui->yMoveTo->setText("Y: "+QString::number(this->optionDialog->getSize().y()*10-point.y()));
}


//main windows close event
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(printerObj->getIsPrinting()){
        event->ignore();
        //Setting parent will keep messagebox in the center of QMainWindow
        msgBox = new QMessageBox(this);
        msgBox->setWindowTitle(tr("Printing in progress"));
        msgBox->setText(tr("Are you sure you want to exit?"));

        QPushButton *yesButton = msgBox->addButton(tr("Yes"), QMessageBox::ActionRole);
        msgBox->addButton(tr("No"), QMessageBox::ActionRole);
        msgBox->exec();

        //If user clicks 'Yes'  button , accept QCloseEvent (Close Window)
        if ((QPushButton*)msgBox->clickedButton() == yesButton)
        {
            event->accept();
        }
    }
    saveSettings();
}


//saving and restoring setting

void MainWindow::saveSettings(){
    QSettings settings("3d-printers", "yarrh");
    settings.clear();
    settings.setValue("lastDir", this->lastOpendDir);
    settings.setValue("XYspeed",ui->speedSpinBox->value()*60);
    settings.setValue("Zspeed",ui->speedZSpinBox->value()*60);
    settings.setValue("mainWindowPos", this->pos());
    settings.setValue("mainWindowSize",this->size());
    settings.setValue("baudRateSelected", ui->baudCombo->currentIndex());
    settings.setValue("showConsole", ui->groupBox_2->isChecked());
    settings.setValue("extrudeLenght", ui->extrudeLenghtSpinBox->value());
    settings.setValue("extrudeSpeed", ui->extrudeSpeedSpinBox->value());
    settings.setValue("monitorTemp", ui->graphGroupBox->isChecked());
    settings.setValue("showTravel", ui->showTravelChkBox->isChecked());
    //auto callibration
    settings.setValue("xStepsPerMm", this->calibrateDialog->getCallibrationsSetting().x());
    settings.setValue("autoCallibrateX", this->calibrateDialog->autoCalibrateX());
    settings.setValue("yStepsPerMm", this->calibrateDialog->getCallibrationsSetting().y());
    settings.setValue("autoCallibrateY", this->calibrateDialog->autoCalibrateY());
    settings.setValue("zStepsPerMm", this->calibrateDialog->getCallibrationsSetting().z());
    settings.setValue("autoCallibrateZ", this->calibrateDialog->autoCalibrateZ());
    //slicer conf
    settings.setValue("slicerDir", this->optionDialog->getSlicerDir());
    settings.setValue("configDir", this->optionDialog->getConfigDir());
    settings.setValue("outputDir", this->optionDialog->getOutputDir());
    //table size
    settings.setValue("sizeX", this->optionDialog->getSize().x());
    settings.setValue("sizeY", this->optionDialog->getSize().y());
    settings.setValue("sizeZ", this->optionDialog->getSize().z());
    //window look
    settings.setValue("font", QApplication::font().toString());
    //write temperature setting
    settings.beginWriteArray("temp1Values");
    for(int i=0; i<ui->t1Combo->count(); i++){
        settings.setArrayIndex(i);
        settings.setValue("value",ui->t1Combo->itemText(i));
        settings.setValue("isSelected", i==ui->t1Combo->currentIndex());
    }
    settings.endArray();

    settings.beginWriteArray("temp3Values");
    for(int i=0; i<ui->hbCombo->count(); i++){
        settings.setArrayIndex(i);
        settings.setValue("value",ui->hbCombo->itemText(i));
        settings.setValue("isSelected", i==ui->hbCombo->currentIndex());
    }
    settings.endArray();

    settings.beginWriteArray("extruders");
    QList< QPair<int, QString > > extruders=this->optionDialog->getExtruders();
    for(int i=0; i<extruders.size(); i++){
        settings.setArrayIndex(i);
        settings.setValue("number",extruders.at(i).first);
        settings.setValue("color", extruders.at(i).second);
    }
    settings.endArray();

    settings.sync();
}

void MainWindow::restoreSettings(){
    QSettings settings("3d-printers", "yarrh");
    this->lastOpendDir = settings.value("lastDir","").toString();
    this->sliceDialog->setLastDir(this->lastOpendDir);
    ui->speedSpinBox->setValue(settings.value("XYspeed").toInt()/60);
    ui->speedZSpinBox->setValue(settings.value("Zspeed").toInt()/60);
    if(settings.value("mainWindowPos").toPoint().x()>QApplication::desktop()->screenGeometry().bottomRight().y() ^ settings.value("mainWindowPos").toPoint().x()>QApplication::desktop()->screenGeometry().bottomRight().y()){
        this->move(QPoint(0,0));
    }
    else{
        this->move(settings.value("mainWindowPos").toPoint());
    }
    this->resize(settings.value("mainWindowSize").toSize());
    ui->baudCombo->setCurrentIndex(settings.value("baudRateSelected").toInt());
    ui->groupBox_2->setChecked(settings.value("showConsole").toBool());
    ui->extrudeLenghtSpinBox->setValue(settings.value("extrudeLenght").toInt());
    ui->extrudeSpeedSpinBox->setValue(settings.value("extrudeSpeed").toInt());
    ui->graphGroupBox->setChecked(settings.value("monitorTemp").toBool());
    ui->showTravelChkBox->setChecked(settings.value("showTravel").toBool());
    //auto callibration
    this->calibrateDialog->setAutoCalibrateX(settings.value("autoCallibrateX",false).toBool());
    this->calibrateDialog->setXStepsPerMm(settings.value("xStepsPerMm",40).toDouble());
    this->calibrateDialog->setAutoCalibrateY(settings.value("autoCallibrateY",false).toBool());
    this->calibrateDialog->setYStepsPerMm(settings.value("yStepsPerMm",40).toDouble());
    this->calibrateDialog->setAutoCalibrateZ(settings.value("autoCallibrateZ",false).toBool());
    this->calibrateDialog->setZStepsPerMm(settings.value("zStepsPerMm",2560).toDouble());
    this->calibrateDialog->setAutoCalibrateE(settings.value("autoCallibrateE",false).toBool());
    this->calibrateDialog->setEStepsPerMm(settings.value("eStepsPerMm",40).toDouble());
    //slicer conf
    this->optionDialog->setSlicerDir(settings.value("slicerDir","").toString());
    this->sliceDialog->updateSlicerPath(settings.value("slicerDir","").toString());
    this->optionDialog->setConfigDir(settings.value("configDir","").toString());
    this->optionDialog->setOutputDir(settings.value("outputDir","").toString());
    this->sliceDialog->updateOutputPath(settings.value("outputDir","").toString());
    //table size
    this->optionDialog->setSize(QVector3D(settings.value("sizeX",20).toInt(),settings.value("sizeY",20).toInt(),settings.value("sizeZ",20).toInt()));
    ui->glWidget->setTableSize(this->optionDialog->getSize().x(),this->optionDialog->getSize().y());
    this->sliceDialog->setTableSize(this->optionDialog->getSize().x(),this->optionDialog->getSize().y());
    ui->headControlWidget->setSize(this->optionDialog->getSize().x(),this->optionDialog->getSize().y());
    ui->headControlWidget->hidePoints(true);
    ui->zSlider->setMaximum(this->optionDialog->getSize().z()*10);
    //window look
    this->optionDialog->setCurrentFont(settings.value("font",QApplication::font().toString()).toString());
    //restore temp1 combo
    int size = settings.beginReadArray("temp1Values");
    int currentIndex=0;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ui->t1Combo->addItem(settings.value("value").toString());
        if(settings.value("isSelected").toBool()){
            currentIndex=i;
        }
    }
    ui->t1Combo->setCurrentIndex(currentIndex);
    settings.endArray();
    //restore temp1 combo
    size = settings.beginReadArray("temp3Values");
    currentIndex=0;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ui->hbCombo->addItem(settings.value("value").toString());
        if(settings.value("isSelected").toBool()){
            currentIndex=i;
        }
    }
    ui->hbCombo->setCurrentIndex(currentIndex);
    settings.endArray();
    //restore extruders
    size = settings.beginReadArray("extruders");
    if(size==0){
        this->optionDialog->addExtruder(qMakePair(0,QString("#eeeeee")));
    }
    currentIndex=0;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        this->optionDialog->addExtruder(qMakePair(settings.value("number").toInt(),settings.value("color").toString()));
    }
    settings.endArray();

}

//writing command to console
void MainWindow::on_outLine_returnPressed()
{
    QMetaObject::invokeMethod(printerObj,"send_now",Qt::QueuedConnection,Q_ARG(QString, ui->outLine->text().toUpper()));
    ui->outLine->clear();
}


//hiding console
void MainWindow::on_groupBox_2_toggled(bool arg1)
{
    if(!arg1){
        ui->groupBox_2->setMaximumHeight(15);
    }
    else{
        ui->groupBox_2->setMaximumHeight(100);
    }
}

void MainWindow::on_fanBtn_toggled(bool on)
{
        if(on){
            ui->fanBtn->setText("Off");
             ui->fanSpinBox->blockSignals(false);
            QMetaObject::invokeMethod(printerObj,"setFan",Qt::QueuedConnection,Q_ARG(int, ui->fanSpinBox->value()));
        }
        else{
            ui->fanBtn->setText("On");
            ui->fanSpinBox->blockSignals(true);
            QMetaObject::invokeMethod(printerObj,"setFan",Qt::QueuedConnection,Q_ARG(int, 0));
        }
}

void MainWindow::on_extrudeBtn_clicked()
{
    QMetaObject::invokeMethod(printerObj,"extrude",Qt::QueuedConnection,Q_ARG(int, ui->extrudeLenghtSpinBox->value()),Q_ARG(int, ui->extrudeSpeedSpinBox->value()));
}

void MainWindow::on_retracktBtn_clicked()
{
    QMetaObject::invokeMethod(printerObj,"retrackt",Qt::QueuedConnection,Q_ARG(int, ui->extrudeLenghtSpinBox->value()),Q_ARG(int, ui->extrudeSpeedSpinBox->value()));
}

void MainWindow::on_actionO_Programie_triggered()
{
    this->aboutWindow->showOnXY(QPoint(this->geometry().center().x()-this->aboutWindow->width()/2,this->geometry().center().y()-this->aboutWindow->height()/2));
}

void MainWindow::on_actionCalibrate_printer_triggered()
{
    this->calibrateDialog->move(QPoint(this->geometry().center().x()-this->calibrateDialog->width()/2,this->geometry().center().y()-this->calibrateDialog->height()/2));
    this->calibrateDialog->show();
}

void MainWindow::on_graphGroupBox_toggled(bool arg1)
{

}

void MainWindow::on_actionOptions_triggered()
{
    this->optionDialog->move(QPoint(this->geometry().center().x()-this->optionDialog->width()/2,this->geometry().center().y()-this->optionDialog->height()/2));
    this->optionDialog->show();
}


void MainWindow::updatadeSize(QVector3D newSize){
    this->sliceDialog->setTableSize(newSize.x(), newSize.y());
    ui->glWidget->setTableSize(newSize.x(), newSize.y());
    ui->headControlWidget->setSize(newSize.x(), newSize.y());
    ui->headControlWidget->hidePoints(ui->headControlWidget->getPointsHidden());
    ui->zSlider->setMaximum(newSize.z()*10);
}

void MainWindow::printFinished(bool value){
    if(value){
        qDebug() << this->durationTime.toString("hh:mm:ss");
        ui->pauseBtn->setEnabled(false);
        ui->axisControlGroup->setEnabled(true);
        ui->printBtn->setEnabled(true);
        ui->progressBar->setHidden(true);
        ui->printBtn->setText(tr("Print"));
        ui->printBtn->setIcon(QIcon(":/imgs/icons/print.png"));

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Print finished in ")+this->durationTime.toString("hh:mm:ss"));
        msgBox.exec();
    }
}

void MainWindow::on_fineUpZBtn_clicked()
{
   QMetaObject::invokeMethod(printerObj,"moveHeadZ",Qt::QueuedConnection,Q_ARG(double, (double)0.1),Q_ARG(int, ui->speedZSpinBox->value()*60), Q_ARG(bool, true));
}

void MainWindow::on_fineDownZBtn_clicked()
{
    QMetaObject::invokeMethod(printerObj,"moveHeadZ",Qt::QueuedConnection,Q_ARG(double, (double)-0.1),Q_ARG(int, ui->speedZSpinBox->value()*60), Q_ARG(bool, true));
}

void MainWindow::on_homeZ_clicked()
{
    QMetaObject::invokeMethod(printerObj,"homeZ",Qt::QueuedConnection);
    ui->zSlider->blockSignals(true);
    ui->zSlider->setValue(0);
    ui->zSlider->blockSignals(false);

}

void MainWindow::on_corseUpZBtn_clicked()
{
    QMetaObject::invokeMethod(printerObj,"moveHeadZ",Qt::QueuedConnection,Q_ARG(double, (double)1.0),Q_ARG(int, ui->speedZSpinBox->value()*60), Q_ARG(bool, true));
    ui->zSlider->blockSignals(true);
    ui->zSlider->setValue(ui->zSlider->value()+1);
    ui->zSlider->blockSignals(false);
}

void MainWindow::on_corseDownZBtn_clicked()
{
    QMetaObject::invokeMethod(printerObj,"moveHeadZ",Qt::QueuedConnection,Q_ARG(double, (double)-1.0),Q_ARG(int, ui->speedZSpinBox->value()*60), Q_ARG(bool, true));
    ui->zSlider->blockSignals(true);
    ui->zSlider->setValue(ui->zSlider->value()-1);
    ui->zSlider->blockSignals(false);
}
