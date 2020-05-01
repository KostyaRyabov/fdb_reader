#include "tools.h"

void Tools::makeEmptyTableFile(QString &tableName, QString &dirPath){
    QFile file(dirPath + tableName + fileFormat);
    if (file.open(QIODevice::WriteOnly)){
        qDebug() << " file.path = " << dirPath + tableName + fileFormat;
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

        out << tableName << 0;

        file.close();
    }
}

QString Tools::InitDB(){
    qDebug() << "Inizialisation database directory path";
    QString dirPath;
    QStringList tableNames = {"planes","company","status","hangar","shedule","way"};
    if (!QDir("database").exists()){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(0, QString::fromUtf8("database folder was not found"),
                              QString::fromUtf8("add existing database folder?"),
                              QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes){
            dirPath = QFileDialog::getExistingDirectory(0,"set database folder","") + "/";

            if (dirPath == "/"){
                QDir().mkdir("database");
                dirPath = QDir::currentPath()+"/database/";

                for (auto &table : tableNames){
                    Tools::makeEmptyTableFile(table, dirPath);
                }
            }else{
                for (auto &table : tableNames){
                    if (!QFile(dirPath + table + fileFormat).exists())
                        Tools::makeEmptyTableFile(table, dirPath);
                }
            }
        }else{
            QDir().mkdir("database");
            dirPath = QDir::currentPath()+"/database/";

            for (auto &table : tableNames){
                Tools::makeEmptyTableFile(table, dirPath);
            }
        }
    }else{
        dirPath = QDir::currentPath()+"/database/";
        for (auto &table : tableNames){
            if (!QFile(dirPath + table + fileFormat).exists())
                Tools::makeEmptyTableFile(table, dirPath);
        }
    }

    qDebug() << " dirPath = " << dirPath;

    return dirPath;
}

Tools::Reader::Reader(QString dirPath){
    this->dirPath = dirPath;
    qDebug() << "       dirPath was added:"<<this->dirPath;
}

Tools::Reader::~Reader(){
    delete fileStream;
    delete file;
}

QString &Tools::Reader::getTableName(){
    return currentTableName;
}

uint& Tools::Reader::getSize(){
    return size;
}

bool Tools::Reader::open(QString fileName, QIODevice::OpenModeFlag flag){
    qDebug() << "       going to open"<<dirPath+fileName+fileFormat;

    if (file != nullptr){
        delete file;
    }

    file = new QFile(dirPath+fileName+fileFormat);

    qDebug()<<"file path ="<<file->fileName();
    if (!file->exists()){
        qDebug() << "file not exists";
        makeEmptyTableFile(fileName, dirPath);
    }else{
        qDebug() << "file exists";
    }

    if (!file->open(flag)){
        QMessageBox msgBox;
        msgBox.setText("Error: could not open file " + fileName + fileFormat + "\r\n"+ file->errorString());
        msgBox.exec();
        return false;
    }else{
        qDebug() << "success";
    }

    if (fileStream != nullptr){
        delete fileStream;
    }

    fileStream = new QDataStream(file);

    if (flag == QIODevice::ReadOnly){
        *fileStream >> currentTableName >> size;

        tmp_obj = essences::getObjectByName(currentTableName);
        qDebug() << "detected"<<currentTableName<<"["<<size<<"]";

        if (!QStringList{"planes","company","status","hangar","shedule","way"}.contains(currentTableName)){
            QMessageBox msgBox;
            msgBox.setText("Error: " + fileName + fileFormat + " is not the database file!");
            msgBox.exec();
            return false;
        }
    }

    return true;
}

void Tools::Reader::close(){
    file->close();
}

bool Tools::Reader::next(){
    qDebug() << "   [left"<<size<<"elements]";
    if (size > 0){
        for (auto i = 0; i < tmp_obj.size(); i++)
            *fileStream >> tmp_obj.at(i);

        size--;
        return true;
    }
    return false;
}

QVariant Tools::Reader::value(int i){
    auto type = tmp_obj.at(i).type();
    if (type == QVariant::DateTime){
        return tmp_obj.at(i).value<QDateTime>();
    }else if (type == QVariant::Time){
        return tmp_obj.at(i).value<QTime>();
    }else if (type == QVariant::UInt){
        return tmp_obj.at(i).value<uint>();
    }else if (type == QVariant::String){
        return tmp_obj.at(i).value<QString>();
    }else{
        return tmp_obj.at(i).value<int>();
    }
}

QVariant::Type Tools::Reader::fieldType(int i){
    if (tmp_obj.size() == 0) return QVariant::Int;
    return tmp_obj.at(i).type();
}

void Tools::Reader::operator<< (QVariant value){
    *fileStream<<value;
}
void Tools::Reader::operator<< (QString value){
    *fileStream<<value;
}
void Tools::Reader::operator<< (int value){
    *fileStream<<value;
}
