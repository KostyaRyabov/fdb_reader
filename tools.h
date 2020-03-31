#ifndef TOOLS_H
#define TOOLS_H

#include <QString>
#include <QFile>
#include "mymodel.h"
#include <QLinkedList>
#include <essences.h>

namespace Tools{
    #define fileFormat ".fdb"

    void makeEmptyTableFile(QString &tableName, QString &dirPath);

    class db{
        Q_OBJECT
    public:
        db(QObject *parent = nullptr);

        bool open(QString dirPath);

        db& operator<<(QLinkedList<essences::o> &data);
        db& operator>>(QLinkedList<essences::o> &data);
    private:
        QString dirPath;
        QLinkedList<essences::o> data;
    };
}

#endif // TOOLS_H
