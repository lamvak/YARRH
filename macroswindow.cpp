#include "macroswindow.h"
#include "ui_macroswindow.h"

MacrosWindow::MacrosWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MacrosWindow)
{
    ui->setupUi(this);
}

MacrosWindow::~MacrosWindow()
{
    delete ui;
}

void MacrosWindow::addMacro(QString name, QString value){
    MacroButton* newMacro = new MacroButton(name,value);
    this->macros.append(newMacro);
    ui->macroList->clear();
    for(int i=0; i< this->macros.size(); i++){
        ui->macroList->addItem(this->macros.at(i)->getName());
    }
    connect(newMacro, SIGNAL(clicked()), this, SLOT(onMacroClick()));
    emit buttonAdded(newMacro);
}

void MacrosWindow::on_addBtn_clicked()
{
    addMacro("New","");
}

void MacrosWindow::on_removeBtn_clicked()
{
    if(ui->macroList->currentRow()>=0 && ui->macroList->currentRow()<this->macros.size()){
        emit buttonRemoved(this->macros.takeAt(ui->macroList->currentRow()));
    }
    ui->macroList->clear();
    for(int i=0; i< this->macros.size(); i++){
        ui->macroList->addItem(this->macros.at(i)->getName());
    }
    if(ui->macroList->count()>0){
        ui->macroList->setCurrentRow(0);
    }
}

void MacrosWindow::on_macroList_currentRowChanged(int currentRow)
{
    if(ui->macroList->currentRow()>=0 && ui->macroList->currentRow()<this->macros.size()){
        ui->macroName->setText(this->macros.at(ui->macroList->currentRow())->getName());
        ui->macroValue->setText(this->macros.at(ui->macroList->currentRow())->getValue());
        ui->groupBox_2->setEnabled(true);
    }
    else{
        ui->groupBox_2->setEnabled(false);
    }
    qDebug() << currentRow;
}

void MacrosWindow::on_saveBtn_clicked()
{
    this->macros.at(ui->macroList->currentRow())->setName(ui->macroName->text());
    ui->macroList->item(ui->macroList->currentRow())->setText(ui->macroName->text());
    this->macros.at(ui->macroList->currentRow())->setValue(ui->macroValue->toPlainText());
}

void MacrosWindow::onMacroClick(){
    MacroButton* sender=(MacroButton*)QObject::sender();
    qDebug() << sender->getName();
    QStringList commands=sender->getValue().split("\n");
    for(int i=0; i<commands.size(); i++){
        emit writeToPrinter(commands.at(i).toUpper());
    }
}
