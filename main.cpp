#include <QtGui/QApplication>
#include <QDebug>
#include <QTextCodec>
#include "./qtsingleapplication.h"



#include "mainwindow.h"

int main(int argc, char *argv[])
{
    //setting string codecs
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName ("UTF-8"));

    QtSingleApplication a("YARRH",argc, argv);
    QString locale = QLocale::system().name();
    QString message;
    //get arguments
    for (int a = 0; a < argc; ++a) {
        message += argv[a];
        message += "-||-";
    }

    if (a.sendMessage(message))
        return 0;
    a.initialize(FALSE);

    if(a.isRunning()){
        return 0;
    }

    QCoreApplication::setOrganizationName("wuflnor");
    // XXX This domain is only for Mac settings.
    QCoreApplication::setOrganizationDomain("wuflnor.github.org");
    QCoreApplication::setApplicationName("YARRH");

    QTranslator translator;
    qDebug() << translator.load(QString(":/yarrh_")+locale);
    a.installTranslator(&translator);
    MainWindow w;
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)),
                        &w, SLOT(processMessage(const QString&)));
    w.show();
    w.processMessage(message);
    return a.exec();
}
