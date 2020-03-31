#include "essences.h"
#include <QDebug>

const char* essences::planes::getName(){
    return "planes";
}

const char* essences::company::getName(){
    return "company";
}

const char* essences::status::getName(){
    return "status";
}

const char* essences::shedule::getName(){
    return "shedule";
}

const char* essences::hangar::getName(){
    return "hangar";
}

const char* essences::way::getName(){
    return "way";
}

int essences::o::size(){
    return args.count();
}

QVariant& essences::o::at(int index){
    if (index < 0 || index >= this->size()) {
        return args[0];
    }

    return args[index];
}

essences::planes::planes(){
    args.append(int());
    args.append(QString());
    args.append(int());
    args.append(int());
    args.append(int());
    args.append(QDateTime());
}

essences::company::company(){
    args.append(int());
    args.append(QString());
}

essences::status::status(){
    args.append(int());
    args.append(QString());
}

essences::hangar::hangar(){
    args.append(int());
    args.append(int());
    args.append(uint());
    args.append(QDateTime());
}

essences::shedule::shedule(){
    args.append(int());
    args.append(int());
    args.append(int());
    args.append(int());
    args.append(int());
    args.append(QDateTime());
}

essences::way::way(){
    args.append(int());
    args.append(QString());
    args.append(uint());
    args.append(uint());
    args.append(QTime());
}


essences::o essences::getObjectByName(QString TypeName){
    if (TypeName == "planes")
        return essences::planes();
    else if (TypeName == "company")
        return essences::company();
    else if (TypeName == "status")
        return essences::status();
    else if (TypeName == "hangar")
        return essences::hangar();
    else if (TypeName == "shedule")
        return essences::shedule();
    else if (TypeName == "way")
        return essences::way();
    else{
        return essences::o();
    }
}

QStringList essences::getHeadersOf(QString TypeName){
    if (TypeName == "planes")
        return {"id", "name", "companyID", "size", "statusID", "dataCreate"};
    else if (TypeName == "company")
        return {"id", "name"};
    else if (TypeName == "status")
        return {"id", "record"};
    else if (TypeName == "hangar")
        return {"id", "planesID", "Reserved", "datatime"};
    else if (TypeName == "shedule")
        return {"id", "planesID", "hangarID", "wayID", "PassangerCount", "datatime"};
    else if (TypeName == "way")
        return {"id", "country", "arrives", "nonstop", "duration"};
    else{
        return {};
    }
}
