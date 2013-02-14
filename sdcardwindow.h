#ifndef SDCARDWINDOW_H
#define SDCARDWINDOW_H

#include <QDialog>
#include <QDebug>
#include <printer.h>
#include <QFileDialog>

namespace Ui {
    class SDCardWindow;
}

class SDCardWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SDCardWindow(QWidget *parent = 0);
    ~SDCardWindow();
    void setPrinter(Printer* printer);
    void setLastDir(QString dir);
private:
    Ui::SDCardWindow *ui;
    Printer* printer;
    QTime startTime;
    QTime eta;
    QString lastDir;
public slots:
    void updateFileList(QStringList files);
private slots:
    void on_selecBtn_clicked();
    void on_uploadBtn_clicked();
    void updateProgress(int progress, int maximum);
signals:
    void sdFile_selected(QString);
};

#endif // SDCARDWINDOW_H
