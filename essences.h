#ifndef ESSENCES_H
#define ESSENCES_H

#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QSqlQuery>
#include <QList>

namespace essences {
    struct o{
        int size();
        static const char* getName();
        QVariant& at(int index);

        protected:
        QList<QVariant> args;
    };

    struct planes:o{
        planes();
        static const char* getName();
    };

    struct company:o{
        company();
        static const char* getName();
    };

    struct status:o{
        status();
        static const char* getName();
    };

    struct hangar:o{
        hangar();
        static const char* getName();
    };

    struct shedule:o{
        shedule();
        static const char* getName();
    };

    struct way:o{
        way();
        static const char* getName();
    };

    essences::o getObjectByName(QString TypeName);
}


#endif // ESSENCES_H

