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

            for (auto &table : tableNames){
                if (!QFile(dirPath + table + fileFormat).exists())
                    Tools::makeEmptyTableFile(table, dirPath);
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

}

bool Tools::Reader::open(QString fileName, QIODevice::OpenModeFlag flag){
    qDebug() << "       going to open"<<dirPath+fileName+fileFormat;
    QStringList tableNames = {"planes","company","status","hangar","shedule","way"};

    if (!tableNames.contains(fileName)){
        QMessageBox msgBox;
        msgBox.setText("Error: " + fileName + fileFormat + " is not the database file!");
        msgBox.exec();
        return false;
    }

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

    if (flag == QIODevice::ReadOnly){
        fileStream = new QDataStream(file);
        *fileStream >> currentTableName >> size;
        qDebug() << "detected"<<currentTableName<<"["<<size<<"]";
        tmp_obj = essences::getObjectByName(currentTableName);
    }

    return true;
}

void Tools::Reader::close(){
    file->close();
}

bool Tools::Reader::next(){
    qDebug() << "   [left"<<size<<"elements]";
    if (size > 0){
        tmpDataRow.clear();

        for (auto i = 0; i < tmp_obj.size(); i++)
            *fileStream >> tmpDataRow;

        size--;
        return true;
    }
    return false;
}

QVariant Tools::Reader::value(int i){
    return tmpDataRow[i];
}

QVariant::Type Tools::Reader::fieldType(int i){
    if (tmp_obj.size() == 0) return QVariant::Int;
    return tmp_obj.at(i).type();
}

void Tools::Reader::operator<< (QVariant &value){
    *fileStream<<value;
}
void Tools::Reader::operator<< (QString &value){
    *fileStream<<value;
}
void Tools::Reader::operator<< (int value){
    *fileStream<<value;
}

QDataStream &operator<<(QDataStream &out, MyModel &model) {
    QModelIndexList selection = model.ui->tableView->selectionModel()->selectedRows();

    out << model.currentTable;

    if (selection.size() > 0){
        qDebug() << "   insert "<<selection.size()<<"selection elements";
        out << selection.size();

        for (auto &index : selection){
            for (int column = 0; column < model.header.size(); column++){
                qDebug() << "       ["<<index.row()<<":"<<column<<"] = "<<model.getRow(index.row())->at(column);
                out << model.getRow(index.row())->at(column);
            }
        }
    }else{
        qDebug() << "   insert"<<model.storage.size()<<"elements";
        out << model.storage.size();

        int row = 0;
        for (auto it = model.storage.begin(); it != model.storage.end(); it++, row++){
            for (int x = 0; x < model.header.size(); x++){
                qDebug() << "       ["<<row<<":"<<x<<"] = "<<it->at(x);
                out << it->at(x);
            }
        }
    }


    return out;
}

QDataStream &operator>>(QDataStream &in, MyModel &model) {
    qDebug() << "----loading----";

    uint size;
    QString TableName;

    in >> TableName >> size;

    qDebug() << "TableName = " << TableName << "; size = " << size;

    if (model.currentTable == TableName){
        qDebug() << "DisableCleaning";
        model.clean_isEnabled = false;
    } else {
        model.loadTable(TableName);
    }

    auto item = essences::getObjectByName(model.currentTable);
    qDebug() << "typeid(item).name() = " << typeid(item).name();

    int row;
    for (uint y = 0; y < size; y++){
        for (int x = 0; x < item.size(); x++){
            in >> item.at(x);
            qDebug()<<"   ["<<x<<":"<<y<<"] = "<<item.at(x);
        }

        row = model.storage.count();
        model.beginInsertRows( QModelIndex(), row, row );
        model.storage << item;
        model.endInsertRows();
    }

    qDebug() << "model.inserted_rows_count += size : " << model.inserted_rows_count<<size;
    model.inserted_rows_count += size;
    if (model.inserted_rows_count > 0){
        model.ui->cancel_bn->setEnabled(true);
        model.ui->commit_bn->setEnabled(true);
    }
    qDebug() << "       res = " << model.inserted_rows_count;
    qDebug() << "----loaded----";

    return in;
}
