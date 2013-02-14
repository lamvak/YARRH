#include "printer.h"

Printer::Printer(QObject *parent)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName ("UTF-8"));
    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    this->portObj = new QextSerialPort("",settings);
    this->isPrinting=false;
    this->isPaused=true;
    this->connectionActive=false;
    this->temperatureTimer = new QTimer(this);
    this->temperatureTimer->setInterval(2000);
    connect(temperatureTimer, SIGNAL(timeout()), this, SLOT(getTemperature()));

    this->readTimer = new QTimer(this);
    this->readTimer->setInterval(5);
    connect(this->readTimer, SIGNAL(timeout()), this, SLOT(readFromPort()));

    connect(this, SIGNAL(newResponce(QString)), this, SLOT(processResponce(QString)));
    connect(this, SIGNAL(clearToSend()), this, SLOT(send_next()));
    curr_pos.setX(0);
    curr_pos.setY(0);
    curr_pos.setZ(0);
    last_bed_temp = 0;
    last_head_temp = 0;
    inBuffer.clear();
    responseBuffer.clear();
    writeNext=true;
    this->updatingFileList=false;
    this->sdPrint=false;
    this->uploadingToSD=false;
}

bool Printer::isConnected(){
    if(this->portObj!=NULL){
        return this->portObj->isReadable();
    }
    else
        return false;
}

bool Printer::getIsPrinting(){
    return this->isPrinting;
}

bool Printer::isPrintingFromSD(){
    return this->sdPrint;
}

bool Printer::connectPort(QString port, int baud){
    emit write_to_console(tr("Connecting..."));
    if(port!=""){
        PortSettings settings = {(BaudRateType)baud, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
        delete this->portObj;
        this->portObj = new QextSerialPort(port,settings,QextSerialPort::Polling);
        if(this->portObj->open(QIODevice::ReadWrite | QIODevice::Unbuffered)){
            emit write_to_console(tr("Printer connected"));
            send_now("M115");
            this->readTimer->start();
            return true;
        }
        else{
           emit write_to_console(tr("Unable to connect"));
           emit connected(false);
           return false;
        }
    }
    else{
        emit write_to_console(tr("Unknown port name"));
        emit connected(false);
        return false;
    }
}

bool Printer::disconnectPort(){
    this->portObj->close();
    this->readTimer->stop();
    emit write_to_console(tr("Printer disconnected"));
    emit connected(false);
    this->connectionActive=false;
    return true;
}

//reading from port, we want to be here as short as posible
void Printer::readFromPort(){
    if(portObj->bytesAvailable()>0){
        emit newResponce(portObj->readLine());
    }
}

//slot starting print
void Printer::startPrint(){
    this->isPrinting=true;
    this->isPaused=false;
    this->lineNum=0;
    this->resendFrom=-1;
    this->sentLines.clear();
    this->writeToPort("M110", -1, true);
    send_next();
}

//homing axis
void Printer::homeX(){
    send_now("G28 X0");
}

void Printer::homeY(){
    send_now("G28 Y0");
}
void Printer::homeZ(){
    send_now("G28 Z0");
}
void Printer::homeAll(){
    send_now("G28");
}

//moving head
void Printer::moveHead(QPoint point, int speed){
    send_now("G1 F"+QString::number(speed));
    send_now("G1 X"+QString::number(point.x())+" Y"+QString::number(point.y()));
}

void Printer::moveHeadZ(double z, int speed, bool relative){
    if(relative){
        send_now("G91");
    }
    send_now("G1 F"+QString::number(speed));
    send_now("G1 Z"+QString::number(z));
    if(relative){
        send_now("G90");
    }
}

//setting fan speed
void Printer::setFan(int percent){
    float value=255*((float)percent/(float)100);
    send_now("M106 S"+QString::number((int)value));
}
//disable stepper
void Printer::disableSteppers(){
    send_now("M18");
}

//adding data to printer buffer
void Printer::loadToBuffer(QStringList buffer, bool clear){
    this->curr_pos.setX(0);
    this->curr_pos.setY(0);
    this->curr_pos.setZ(0);
    if(clear){
        this->mainQuery.clear();
    }
    this->mainQuery.append(buffer);
    this->sdPrint=false;
}

void Printer::setMonitorTemperature(bool monitor){
    this->monitorTemperature=monitor;
    if(monitor){
        this->temperatureTimer->start();
    }
    else{
        this->temperatureTimer->stop();
    }
}

void Printer::getTemperature(){
    if(this->isConnected()){
        if(!this->uploadingToSD){
            send_now("M105");
        }
        if(this->sdPrint){
            send_now("M27");
        }
    }
}

void Printer::setTemp1(int temp){
    send_now("M104 S"+QString::number(temp));
}

void Printer::setTemp3(int temp){
     send_now("M140 S"+QString::number(temp));
}

void Printer::extrude(int lenght, int speed){
    send_now("G91");
    send_now("G1 E"+QString::number(lenght)+" F"+QString::number(speed*60));
    send_now("G90");
}

void Printer::retrackt(int lenght, int speed){
    send_now("G91");
    send_now("G1 E"+QString::number(lenght*-1)+" F"+QString::number(speed*60));
    send_now("G90");
}


//calculate checksum of command
QString Printer::checkSum(QString command){
    int cs = 0;
    int i = 0;
    if(command.size()>0){
        for(i = 0;i<command.size(); i++){
            if(command.at(i)=='*'){
                break;
            }
           cs = cs ^ command.at(i).toAscii();
        }
        cs &= 0xff;
    }
    else{
        qDebug() << "Error: empty command:" << command;
    }
    return QString::number(cs);
}

//sending command with checksum checking
void Printer::send(QString command){
    if(this->isPrinting){
        this->mainQuery.append(command);
    }
    else{
        writeToPort(command,this->lineNum,true);
        this->lineNum++;
    }
}

//ending without checksum checking
void Printer::send_now(QString command){
    if(this->isPrinting){
        this->priQuery.append(command);
    }
    else{
        writeToPort(command);
    }
}

//sending next line from buffer
void Printer::send_next(){
    //if printer inst connected then return
    if(!this->isConnected()){
        return;
    }
    //resend commands
    if(this->resendFrom<this->lineNum && this->resendFrom>-1){
        writeToPort(this->sentLines.at(this->resendFrom),this->resendFrom,false);
        this->resendFrom++;
        return;
    }
    this->resendFrom=-1;
    //send all commands from pri query
    while(this->priQuery.size()>0){
        writeToPort(this->priQuery.takeFirst());
        return;
    }
    //if there are lines to send in main buffer
    if(this->isPrinting && this->mainQuery.size()>0){
        writeToPort(this->mainQuery.takeFirst(),this->lineNum,true);
        emit progress(this->mainQuery.size(),-1);
        this->lineNum++;
    }
    else{
        if(this->isPrinting){
            if(!this->sdPrint){
                emit printFinished(true);
            }
        }
        this->isPrinting=false;
    }
}

//writing to port
int Printer::writeToPort(QString command,int lineNum, bool calcCheckSum){
    //analizing command for position tracking
    QStringList args=command.split(" ");

    if(command.contains("M104") || command.contains("M109")){
        emit settingTemp1(command.mid(command.lastIndexOf("S")+1,command.length()-command.lastIndexOf("S")+1).toDouble());
    }
    if(command.contains("M140") || command.contains("M190")){
        emit settingTemp3(command.mid(command.lastIndexOf("S")+1,command.length()-command.lastIndexOf("S")+1).toDouble());
    }

    if(command.contains("G1") || command.contains("G0")){
        for(int i=0 ;i <args.size(); i++){
            if(args.at(i).contains("X"))
                curr_pos.setX(args.at(i).mid(1).toDouble());
            if(args.at(i).contains("Y"))
                curr_pos.setY(args.at(i).mid(1).toDouble());
            if(args.at(i).contains("Z")){
                curr_pos.setZ(args.at(i).mid(1).toDouble());
            }
        }
        emit currentPosition(curr_pos);    
    }

    if(this->isConnected()){
        if(calcCheckSum){
            command=QString("N")+QString::number(lineNum)+QString(" ")+command;
            command=command+QString("*")+checkSum(command);
            if(!command.contains("M110")){
                this->sentLines.append(command);
            }
            else{
                this->lineNum=0;
            }
//            int temp=rand()%10;
//            if(temp==0){
//                command.replace(5,1,"b");
//                qDebug() << "dodaje blad do" << command;
//            }
        }
        int bytes_written =portObj->write(command.toAscii()+"\n");
        emit write_to_console(QString("Sending: "+command.toAscii()+"\n").replace("\n","\\n").replace("\r","\\r"));
        emit write_to_console("Written: "+QString::number(bytes_written)+" bytes");
        writeNext=false;

    }
    else{
        write_to_console(tr("Printer offline"));
        return -1;
    }
    return 0;
}

//proper parsing of responce
void Printer::processResponce(QString responce){
    if(!this->connectionActive){
        this->connectionActive=true;
        emit connected(true);
    }
    //add data to buffer
    inBuffer.append(responce);

    //send full responces to fifo
    if(inBuffer.contains("\n")){
        responseBuffer.append(inBuffer.left(inBuffer.lastIndexOf("\n")+1).split("\n",QString::SkipEmptyParts));
    }
    //clear buffer
    inBuffer.remove(0,inBuffer.lastIndexOf("\n")+1);

    //proccess all responces from fifo
    while(responseBuffer.size()>0){
        QString lastResponse=responseBuffer.takeFirst();
        emit write_to_console("Got responce: "+ lastResponse);
        //if its temp status
        if(lastResponse.contains("T:") || lastResponse.contains("B:")){
            bool ok;

            double bed_temp = lastResponse.mid(lastResponse.indexOf("B:")+2,lastResponse.indexOf(" ",lastResponse.indexOf("B:"))-lastResponse.indexOf("B:")-2).toDouble(&ok);
            if(ok){
                last_bed_temp=bed_temp;
            }
            double head_temp = lastResponse.mid(lastResponse.indexOf("T:")+2,lastResponse.indexOf(" ",lastResponse.indexOf("T:"))-lastResponse.indexOf("T:")-2).toDouble(&ok);
            if(ok){
                last_head_temp=head_temp;
            }
            emit currentTemp(last_head_temp,0.0,last_bed_temp);
        }
        //if its something about sd card
        if(lastResponse.contains("Begin file list")){
            this->sdFiles.clear();
            this->updatingFileList=true;
        }
        if(this->updatingFileList){
            this->sdFiles.append(lastResponse.toLower());
        }
        if(lastResponse.contains("End file list")){
            this->updatingFileList=false;
            this->sdFiles.removeFirst();
            this->sdFiles.removeLast();
            emit SDFileList(this->sdFiles);
        }
        if(lastResponse.contains("SD printing byte")){
            QString prog=lastResponse.right(lastResponse.length()-lastResponse.lastIndexOf(" ")-1);
            QStringList values = prog.split("/");
            emit progress(values.at(0).toInt(), values.at(1).toInt());
            if(values.at(0).toInt()==values.at(1).toInt()){
                emit printFinished(true);
                this->sdPrint=false;
            }
        }
        //if its response for position command
        //if we are printing then continue
        if(lastResponse.startsWith("ok")){
            if(!this->isPaused){
                emit clearToSend();
            }
        }
        //if its error
        else if(lastResponse.startsWith("Error")){

        }
        //resend line
        else if(lastResponse.toLower().startsWith("resend") || lastResponse.startsWith("rs")){
            bool ok;
            int toResend;
           // toResend=QString::number(lastResponse.toLower().remove(QRegExp("[\\D]")));
           toResend=lastResponse.toLower().remove(QRegExp("[\\D]")).toInt(&ok);
           if(ok){
               this->resendFrom=toResend;
               if(!this->isPaused){
                   emit clearToSend();
               }
           }
           else{
               emit write_to_console(tr("something is horribly wrong"));
           }
        }
    }
}

void Printer::pausePrint(bool pause){
    this->isPaused=pause;
    this->isPrinting=!pause;
    if(!pause){
        send_next();
    }
}

//SD stuff

void Printer::getSDFilesList(){
    this->sdFiles.clear();
    if(!this->isPrinting){
        send_now("M21");
        send_now("M20");
    }
}

void Printer::uploadToSD(QString filename, QStringList data){
    this->mainQuery.clear();
    this->mainQuery.append(data);
    send_now("M28 "+filename);
    this->uploadedFileName=filename;
    this->uploadingToSD=true;
    while(mainQuery.size()>0){
        send_now(mainQuery.takeFirst());
        emit uploadProgress(mainQuery.size(), -1);
    }
    send_now("M29 "+filename);
    this->uploadingToSD=false;
    getSDFilesList();
}

void Printer::selectSDFile(QString filename){
    send_now("M23 "+filename);
    this->sdPrint=true;
}

void Printer::startResumeSdPrint(){
    send_now("M24");
}

void Printer::pauseSDPrint(){
    send_now("M25");
}

void Printer::set_isSdPrinting(bool value){
    this->sdPrint=value;
}
