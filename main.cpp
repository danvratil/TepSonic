#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("TepSonic");
    a.setOrganizationDomain("ProgDan Soft");
    a.setApplicationVersion("0.6");
    MainWindow w;
    w.show();
    return a.exec();
}
