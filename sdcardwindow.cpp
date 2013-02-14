#include "sdcardwindow.h"
#include "ui_sdcardwindow.h"

SDCardWindow::SDCardWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDCardWindow)
{
    ui->setupUi(this);
    ui->progressBar->hide();
}

SDCardWindow::~SDCardWindow()
{
    delete ui;
}

void SDCardWindow::setLastDir(QString dir){
    this->lastDir=dir;
}

void SDCardWindow::updateFileList(QStringList files){
    ui->fileList->clear();
    ui->fileList->addItems(files);
}

void SDCardWindow::setPrinter(Printer *printer){
    this->printer=printer;
}

void SDCardWindow::on_selecBtn_clicked()
{
    if(ui->fileList->selectedItems().size()>0){
        printer->selectSDFile(ui->fileList->selectedItems().at(0)->text());
        this->hide();
        emit sdFile_selected(ui->fileList->selectedItems().at(0)->text());
    }
}

void SDCardWindow::on_uploadBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), this->lastDir, tr("GCODE files (*.g *.gcode)"));

    if(fileName!=""){
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        //buffer filecontent
        QString fileContent=file.readAll();
        file.close();
        fileName=fileName.right(fileName.length()-fileName.lastIndexOf("/")-1);
        fileName=fileName.left(fileName.indexOf(".")).toLower();
        fileName.replace(" ","_");
        fileName.truncate(8);
        qDebug() << fileName;
        //split buffer to lines
        QStringList gcodes=fileContent.split("\n");
        ui->progressBar->setMaximum(gcodes.size());
        ui->progressBar->show();
        this->startTime=QTime::currentTime();
        QMetaObject::invokeMethod(printer,"uploadToSD",Qt::DirectConnection,Q_ARG(QString, fileName+".g"), Q_ARG(QStringList, gcodes));
    }
}

void SDCardWindow::updateProgress(int progress, int maximum){
    if(progress==0){
        ui->progressBar->hide();
    }
    ui->progressBar->setValue(ui->progressBar->maximum()-progress);
    qApp->processEvents();
    //calculating ETA
    qreal linesPerMSec=(qreal)ui->progressBar->value()/(qreal)startTime.msecsTo(QTime::currentTime());
    this->eta=QTime(0,0,0).addMSecs((qint64)((qreal)progress/linesPerMSec));

    ui->progressBar->setFormat("%p% "+this->eta.toString("hh:mm:ss"));

}
