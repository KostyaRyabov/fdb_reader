#include "mymodel.h"

#include <QTableView>
#include <QString>
#include <QSqlField>
#include <QCheckBox>
#include <QPair>
#include <qpair.h>
#include <QDebug>
#include <QRegularExpression>
#include <QCheckBox>
#include "essences.h"
#include <QStyledItemDelegate>





MyModel::MyModel( QObject* parent ) : QAbstractTableModel( parent )
{

}

MyModel::~MyModel()
{
    storage.clear();
    dictionaries.clear();
    change_list.clear();
}

void MyModel::bindView(Ui::MainWindow *ui){
    this->ui = ui;
}

essences::o* MyModel::getRow(int i) const{
    for (auto it = storage.begin(); it != storage.end(); i--, it++){
        if (i <= 0){
            return const_cast<essences::o*>(&(*it));
        }
    }

    return nullptr;
}

int MyModel::rowCount( const QModelIndex& parent ) const {
    Q_UNUSED( parent )
    return storage.count();
}

int MyModel::columnCount( const QModelIndex& parent ) const {
    Q_UNUSED( parent )

    if (storage.isEmpty()) return 0;

    return const_cast<essences::o*>(&storage.first())->size();
}

QVariant MyModel::data( const QModelIndex& index, int role ) const {
    if(!index.isValid()){
        return QVariant();
    }

    switch (role) {
    case Qt::BackgroundRole:
        if (index.row() >= storage.size() - inserted_rows_count)
            return QVariant(QColor(235,235,235));

        if (change_list.contains(index.row())){
            return QVariant(QColor(255,175,175));
        }
        return QVariant(QColor(255,255,255));
    case Qt::DisplayRole:
    case Qt::EditRole:
        return getRow(index.row())->at(index.column());
    default:
        return QVariant();
    }
}

bool MyModel::setData( const QModelIndex& index, const QVariant& value, int role ) {
    if( !index.isValid() || role != Qt::EditRole || storage.count() <= index.row() ) {
        return false;
    }

    if (role == Qt::EditRole){
        if (index.row() < storage.size() - inserted_rows_count){
            if (change_list.contains(index.row())){
                if (change_list[index.row()].contains(index.column())){
                    if (change_list[index.row()][index.column()] == value){
                        change_list[index.row()].remove(index.column());
                    }
                }else{
                    change_list[index.row()][index.column()] = (*getRow(index.row())).at(index.column());
                }
                if (change_list[index.row()].isEmpty()){
                    change_list.remove(index.row());
                    if (change_list.isEmpty()){
                        ui->commit_bn->setEnabled(false);
                        ui->cancel_bn->setEnabled(false);
                    }
                }
            }else{
                if ((*getRow(index.row())).at(index.column()) != value){
                    if (change_list.isEmpty()){
                        ui->commit_bn->setEnabled(true);
                        ui->cancel_bn->setEnabled(true);
                    }
                    change_list[index.row()][index.column()] = (*getRow(index.row())).at(index.column());
                }
            }
        }

        (*getRow(index.row())).at(index.column()) = value;
    }

    emit dataChanged( index, index );
    return true;
}

Qt::ItemFlags MyModel::flags( const QModelIndex& index ) const {
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.column() == 0){
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void MyModel::clear(){
    beginResetModel();

    ui->commit_bn->setEnabled(false);
    ui->cancel_bn->setEnabled(false);
    ui->remove_bn->setEnabled(false);
    ui->save_bn->setEnabled(false);
    ui->add_bn->setEnabled(false);

    ui->save_bn->setText("save (all)");

    currentTable.clear();
    inserted_rows_count = 0;
    change_list.clear();

    storage.clear();

    for (int i = 0; i < header.count(); i++) {
        ui->tableView->setItemDelegateForColumn(i, nullptr);
    }

    while (!delegateForColumn.isEmpty()){
        delete delegateForColumn.takeLast();
    }

    header.clear();
    dictionaries.clear();

    endResetModel();
}

void MyModel::getHeader(QStringList list){
    qDebug() << "get headers";
    beginInsertColumns(QModelIndex(), 0, list.size() - 1);
    header = list;
    insertColumns(0,list.size());
    endInsertColumns();

    QRegularExpression re("ID$");
    Tools::Reader reader(dirPath);

    for (int i = 0; i < header.size(); i++){
        if (header[i].contains(re)){
            delegateForColumn.append(new ComboBoxDelegate(i, this));
            ui->tableView->setItemDelegateForColumn(i, delegateForColumn.last());

            QString tableName = header[i].mid(0, header[i].size() - 2);

            dictionaries[header[i]].first.append(0);
            dictionaries[header[i]].second.append(" --- ");

            if (reader.open(tableName, QIODevice::ReadOnly)){
                qDebug() << "dic:"<<header[i]<<"is loading";
                while(reader.next()){
                    dictionaries[header[i]].first.append(reader.value(0).toInt());

                    if (header[i] == "hangarID"){
                        dictionaries[header[i]].second.append(reader.value(0).toString() + " [" + dictionaries[header[1]].second[dictionaries[header[1]].first.indexOf(reader.value(1).toInt())] +"]");
                    }else{
                        dictionaries[header[i]].second.append(reader.value(1).toString());
                    }

                    qDebug() << "   "<<dictionaries[header[i]].first.last()<<dictionaries[header[i]].second.last();
                }

                reader.close();
            }
        }

        // добавить чекбокс
        else if (reader.fieldType(i) == QVariant::UInt){
            delegateForColumn.append(new CheckBoxDelegate(this));
            ui->tableView->setItemDelegateForColumn(i, delegateForColumn.last());
        }
    }
}


QVariant MyModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if( role != Qt::DisplayRole ) {
        return QVariant();
    }

    if( orientation == Qt::Vertical ) {
        return section + 1;
    }

    if (section <= header.count())
    {
        return header[section];
    }

    return QVariant();
}

void MyModel::AddEmptyRow(){
    qDebug() << "adding new item ["<<currentTable<<"]";

    auto row = storage.count();
    beginInsertRows( QModelIndex(), row, row );

    auto item = essences::getObjectByName(currentTable);
    if (storage.size()>0){
        item.at(0) = storage.last().at(0).toInt() + 1;
    }else{
        item.at(0) = 1;
    }
    storage << item;

    inserted_rows_count++;

    endInsertRows();

    ui->save_bn->setEnabled(true);
    ui->commit_bn->setEnabled(true);
    ui->cancel_bn->setEnabled(true);
}

void MyModel::reloadTable(){
    clean_isEnabled = true;
    loadTable(currentTable);
}

void MyModel::loadTable(QString TableName){
    if (TableName == "planes")
        select("planes");
    else if (TableName == "company")
        select("company");
    else if (TableName == "status")
        select("status");
    else if (TableName == "hangar")
        select("hangar");
    else if (TableName == "shedule")
        select("shedule");
    else if (TableName == "way")
        select("way");
    else{
        clear();
    }
}

bool MyModel::select(QString tableName){
    qDebug() << "select"<<tableName;

    if (clean_isEnabled){
        qDebug() << "   clear";
        clear();
    }

    currentTable = tableName;
    auto item = essences::getObjectByName(tableName);

    if (clean_isEnabled){
        qDebug() << "   getHeader";
        getHeader(essences::getHeadersOf(tableName));

        qDebug() << "   change title";
        emit setTitle(currentTable);
    } else {
        clean_isEnabled = true;
    }

    qDebug() << "   create reader (by "<<dirPath+tableName+fileFormat<<")";
    Tools::Reader Reader(dirPath);

    if (Reader.open(tableName, QIODevice::ReadOnly)){
        int i = 0, row;

        qDebug() << "   start reading";
        while (Reader.next()){
            row = storage.count();
            beginInsertRows( QModelIndex(), row, row );

            for (i = 0; i < item.size(); i++) {
                item.at(i) = Reader.value(i);
                qDebug() << "       ["<<row<<":"<<i<<"] = "<<item.at(i);
            }

            storage << item;
            endInsertRows();
        }

        Reader.close();

        ui->add_bn->setEnabled(true);

        if (storage.size() > 0)
            ui->save_bn->setEnabled(true);
    }

    return 1;
}

bool MyModel::UpdateData(){
    qDebug() << "UpdateData";
    Tools::Reader reader(dirPath);

    if (reader.open(currentTable, QIODevice::WriteOnly)){
        qDebug() << "currentTable = "<<currentTable<<"; storage.size() = "<<storage.size();

        reader << currentTable;
        reader << storage.size();

        qDebug() << "parameters installed";

        for (auto it = storage.begin(); it != storage.end(); it++){
            for (int x = 0; x < header.size(); x++){
                reader << it->at(x);
            }
        }

        reader.close();
        return true;
    }

    return false;
}

bool MyModel::RemoveData(){
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

    Tools::Reader reader(dirPath);
    if (reader.open(currentTable, QIODevice::WriteOnly)){
        int row = 0;

        reader << currentTable;
        reader << storage.size() - selection.size();

        auto it_s = selection.begin();

        for (auto it = storage.begin(); it != storage.end(); it++, row++){
            if (it_s != selection.end()){
                qDebug() << "(it_s->row() != row) : " <<it_s->row() <<"!="<< row;
                if (it_s->row() != row){
                    for (int x = 0; x < header.size(); x++){
                        reader << it->at(x);
                    }
                }else{
                    it_s++;
                }
            }else{
                qDebug() << "row : " << row;
                for (int x = 0; x < header.size(); x++){
                    reader << it->at(x);
                }
            }
        }

        reader.close();
        return true;
    }
    return false;
}

bool MyModel::saveData(){
    qDebug() << "saving data";
    QString path = QFileDialog::getSaveFileName(0,"Saving File", "", QString("*")+QString(fileFormat));

    qDebug() << "path ="<< path;

    path = path.mid(0,path.size()-QString(fileFormat).length());
    if (!path.isEmpty()){
        QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

        Tools::Reader reader("");
        if (reader.open(path, QIODevice::WriteOnly)){
            qDebug() << "save"<<currentTable<<selection.size();
            reader << currentTable;

            if (selection.size() > 0){
                reader << selection.size();

                for (auto index = selection.begin(); index != selection.end(); index++){
                    for (int x = 0; x < header.size(); x++){
                        reader << getRow(index->row())->at(x);
                    }
                }
            }else{
                reader << storage.size();

                for (auto it = storage.begin(); it != storage.end(); it++){
                    for (int x = 0; x < header.size(); x++){
                        reader << it->at(x);
                    }
                }
            }



            reader.close();
            return true;
        }
        return false;
    }
    return false;
}

void MyModel::saveDirPath(QString path){
    dirPath = path;
    ui->DirPath->setText(dirPath);
}

bool MyModel::loadData(){
    qDebug() << "----loading----";

    QString path = QFileDialog::getOpenFileName(0,"Opening File", "", "*.fdb");

    qDebug() << "path for loading ="<<path;

    path = path.mid(0,path.size()-QString(fileFormat).length());

    if (!path.isEmpty()){
        Tools::Reader reader("");
        if (reader.open(path, QIODevice::ReadOnly)){
            int row = 0;

            qDebug() << "load"<<reader.getTableName()<<reader.getSize();

            if (reader.getTableName() == currentTable){
                //уже открыта таблица и не нужно вносить изменений
                qDebug() << "DisableCleaning";
                clean_isEnabled = false;
            } else {
                loadTable(reader.getTableName());
            }

            auto item = essences::getObjectByName(currentTable);

            qDebug() << "model.inserted_rows_count += size : " << inserted_rows_count<<reader.getSize();
            inserted_rows_count += reader.getSize();
            if (inserted_rows_count > 0){
                ui->cancel_bn->setEnabled(true);
                ui->commit_bn->setEnabled(true);
            }

            int count = (storage.size()>0)?storage.last().at(0).toInt():1;

            while (reader.next()){
                item.at(0) = count++;
                qDebug() << "       ["<<0<<"] : "<<count++;

                for (int x = 1; x < header.size(); x++){
                    item.at(x) = reader.value(x);
                    qDebug() << "       ["<<x<<"] : "<<item.at(x);
                }

                row = storage.count();
                beginInsertRows( QModelIndex(), row, row );
                storage << item;
                qDebug() << "           (loaded)";
                endInsertRows();
            }

            qDebug() << "       res = " << inserted_rows_count;
            qDebug() << "----loaded----";

            reader.close();
            return true;
        }
        return false;
    }
    return false;
}
