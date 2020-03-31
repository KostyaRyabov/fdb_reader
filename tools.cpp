#include "tools.h"

void Tools::makeEmptyTableFile(QString &tableName, QString &dirPath){
    QFile file(dirPath + tableName + fileFormat);
    if (file.open(QIODevice::WriteOnly)){
        qDebug() << " file.path = " << dirPath + tableName + fileFormat;
        QDataStream out(&file);

        out << tableName << 0;

        file.close();
    }
}

void loadTableData(QString tableName){

}

bool save(MyModel &model) {
    QDataStream out;
    QModelIndexList selection = model.ui->tableView->selectionModel()->selectedRows();

    out << model.currentTable;

    if (selection.size() > 0){
        out << selection.size();

        for (auto &index : selection){
            for (int column = 0; column < model.header.size(); column++){
                out << model.getRow(index.row())->at(column);
            }
        }
    }else{
        out << model.storage.size();

        for (auto it = model.storage.begin(); it != model.storage.end(); it++){
            for (int x = 0; x < model.header.size(); x++){
                out << it->at(x);
            }
        }
    }


    return out;
}

bool load(MyModel &model) {
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
