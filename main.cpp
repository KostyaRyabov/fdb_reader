#include "mainwindow.h"
#include <QApplication>

#include <QFile>
#include <QFileDialog>

#include <QtGui>
#include <tools.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    QStringList tableNames = {"planes","company","status","hangar","shedule","way"};
    QString dirPath;

    if (!QDir("database").exists()){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(0, QString::fromUtf8("database folder was not found"),
                              QString::fromUtf8("add existing database folder?"),
                              QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes){
            dirPath = QFileDialog::getExistingDirectory(0,"set database folder","") + "/";

            for (auto &table : tableNames){
                if (!QFile(dirPath + table + fileFormat).exists())
                    Tools::makeEmptyTableFile(table, dirPath);
            }
        }else{
            QDir folder;
            folder.mkdir("database");

            dirPath = folder.path() + "/";

            for (auto &table : tableNames){
                Tools::makeEmptyTableFile(table, dirPath);
            }
        }
    }else{
        dirPath = QDir().path() + "/database/";
        for (auto &table : tableNames){
            if (!QFile(dirPath + table + fileFormat).exists())
                Tools::makeEmptyTableFile(table, dirPath);
        }
    }

    qDebug() << " dirPath = " << dirPath;

    w.show();
    return a.exec();
}
