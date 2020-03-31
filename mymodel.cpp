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
}

void MyModel::bindView(Ui::MainWindow *ui){
    this->ui = ui;
}

bool MyModel::saveData(){
    QString path = QFileDialog::getSaveFileName(0,"Saving File", "", "*.fdb");

    if (!path.isEmpty()){
        QFile file(path);

        if (file.open(QIODevice::WriteOnly)){
            QDataStream stream(&file);
            stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);
            stream << *this;

            if (stream.status() != QDataStream::Ok){
                QMessageBox msgBox;
                msgBox.setText("ошибка записи");
                msgBox.exec();

                file.close();
                return false;
            }else{
                file.close();
                return true;
            }
        }

        file.close();
        return true;
    }
    return false;
}

void MyModel::saveDirPath(QString path){
    dirPath = path;
}

bool MyModel::loadData(){
    QString path = QFileDialog::getOpenFileName(0,"Opening File", "", "*.fdb");

    if (!path.isEmpty()){
        QFile file(path);

        qDebug() << "file.fileName() = " << file.fileName();

        if (file.open(QIODevice::ReadOnly)){
            QDataStream stream(&file);
            stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

            qDebug() << " 1 : storage.size() = " << storage.size();
            stream >> *this;
            qDebug() << " 2 : storage.size() = " << storage.size();

            if (stream.status() != QDataStream::Ok){
                QMessageBox msgBox;
                msgBox.setText("ошибка чтения");
                msgBox.exec();

                file.close();
                return false;
            }else{
                qDebug() << "inserted_rows_count = " << inserted_rows_count;

                file.close();
                return true;
            }
        }
        file.close();
        return true;
    }
    return false;
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
        return Qt::ItemIsEnabled;
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

            reader.open(tableName, QIODevice::ReadOnly);
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
    auto row = storage.count();
    beginInsertRows( QModelIndex(), row, row );

    auto item = essences::getObjectByName(currentTable);
    item.at(0) = storage.last().at(0).toInt() + 1;
    storage << item;

    inserted_rows_count++;

    endInsertRows();

    ui->commit_bn->setEnabled(true);
    ui->cancel_bn->setEnabled(true);
}

bool MyModel::UpdateData(){
    Tools::Reader reader(dirPath);
    reader.open(currentTable, QIODevice::WriteOnly);
    int row = 0;

    for (auto it = storage.begin(); it != storage.end(); it++, row++){
        if (change_list.contains(row)){
            auto item = change_list[row];
            for (int x = 0; x < header.size(); x++){
                if (item.contains(x)){
                    reader << item[x];
                }else{
                    reader << it->at(x);
                }
            }
        }else{
            for (int x = 0; x < header.size(); x++){
                reader << it->at(x);
            }
        }
    }

    reader.close();
    return true;
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
    Reader.open(tableName, QIODevice::ReadOnly);

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

    if (storage.size() > 0)
        ui->add_bn->setEnabled(true);

    ui->save_bn->setEnabled(true);

    return 1;
}

bool MyModel::InsertData(){
    Tools::Reader reader(dirPath);
    reader.open(currentTable, QIODevice::WriteOnly);
    int row = 0;

    for (auto it = storage.begin(); it != storage.end(); it++, row++){
        for (int x = 0; x < header.size(); x++){
            reader << it->at(x);
        }
    }

    reader.close();
    return true;
}

bool MyModel::RemoveData(){
    QStringList query_str;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

    Tools::Reader reader(dirPath);
    reader.open(currentTable, QIODevice::WriteOnly);
    int row = 0;

    auto it_s = selection.begin();

    for (auto it = storage.begin(); it != storage.end(); it++, row++){
        if (it_s != selection.end()){
            if (it_s->row() >= row){
                for (int x = 0; x < header.size(); x++){
                    reader << it->at(x);
                }
            }else{
                it_s++;
            }
        }else{
            for (int x = 0; x < header.size(); x++){
                reader << it->at(x);
            }
        }
    }

    reader.close();
    return true;
}
