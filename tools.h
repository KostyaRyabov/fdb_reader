#ifndef TOOLS_H
#define TOOLS_H

#include <QString>
#include <QFile>
#include "mymodel.h"
#include <QLinkedList>
#include <essences.h>
#include <QVariant>


namespace Tools{
    #define fileFormat ".fdb"

    void makeEmptyTableFile(QString &tableName, QString &dirPath);
    QString InitDB();

    class Reader{
    public:
        Reader(QString dirPath);
        ~Reader();

        bool open(QString fileName, QIODevice::OpenModeFlag flag);
        void close();

        bool next();
        QVariant value(int i);
        QVariant::Type fieldType(int i);

        void operator<< (QVariant &value);
        void operator<< (QString &value);
        void operator<< (int value);
    private:
        QString dirPath;
        essences::o tmp_obj;
        QList<QVariant> tmpDataRow;
        uint size = 0;
        QDataStream *fileStream = nullptr;
        QString currentTableName;
        QFile *file = nullptr;
    };
}

#endif // TOOLS_H
