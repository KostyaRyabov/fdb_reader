#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    w.db = QSqlDatabase::addDatabase("QODBC3");
    w.db.setDatabaseName("DRIVER={SQL Server};"
                         "SERVER=DESKTOP-QHPKPIB;"
                         "DATABASE=DB;Trusted_Connection=yes;");

    if(!w.db.open())
    {
        QMessageBox msgBox;
        msgBox.setText("ERROR: " + QSqlError(w.db.lastError()).text());
        msgBox.exec();
    }
    else{
        w.show();
    }

    return a.exec();
}
