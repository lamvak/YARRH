#ifndef MACROSWINDOW_H
#define MACROSWINDOW_H

#include <QDialog>
#include <macrobutton.h>
#include <QHash>
#include <QDebug>

namespace Ui {
    class MacrosWindow;
}

class MacrosWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MacrosWindow(QWidget *parent = 0);
    ~MacrosWindow();
    void addMacro(QString name, QString value);
    QList<MacroButton*> macros;
private slots:
    void on_addBtn_clicked();

    void on_removeBtn_clicked();

    void on_macroList_currentRowChanged(int currentRow);

    void on_saveBtn_clicked();
    void onMacroClick();
signals:
    void buttonAdded(MacroButton*);
    void buttonRemoved(MacroButton*);
    void writeToPrinter(QString);
private:
    Ui::MacrosWindow *ui;
};

#endif // MACROSWINDOW_H
