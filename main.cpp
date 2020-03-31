#include "mainwindow.h"
#include <QApplication>

#include <QFile>
#include <QFileDialog>

#include <QtGui>

Q_DECLARE_METATYPE(QLinkedList<essences::o>);

int main(int argc, char *argv[])
{
    qDebug() << "main()";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
