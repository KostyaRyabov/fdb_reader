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


QDataStream &operator<<(QDataStream &out, MyModel &model) {
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

void MyModel::bindDataBase(QSqlDatabase &db){
    database = &db;
}

void MyModel::getHeader(QSqlRecord &r){
    int count = r.count();
    beginInsertColumns(QModelIndex(), 0, count - 1);
    for (int i = 0; i < count; i++) {
        header.append(r.field(i).name());
    }

    insertColumns(0,count);
    endInsertColumns();

    QSqlQuery query(*database);
    QRegularExpression re("ID$");

    for (int i = 0; i < header.size(); i++){
        if (header[i].contains(re)){
            delegateForColumn.append(new ComboBoxDelegate(i, this));
            ui->tableView->setItemDelegateForColumn(i, delegateForColumn.last());

            QString tableName = header[i].mid(0, header[i].size() - 2);

            dictionaries[header[i]].first.append(0);
            dictionaries[header[i]].second.append(" --- ");

            if (query.exec("SELECT * from " + tableName)){
                while (query.next()){
                    dictionaries[header[i]].first.append(query.value(0).toInt());

                    if (header[i] == "hangarID"){
                        dictionaries[header[i]].second.append(query.value(0).toString() + " [" + dictionaries[header[1]].second[dictionaries[header[1]].first.indexOf(query.value(1).toInt())] +"]");
                    }else{
                        dictionaries[header[i]].second.append(query.value(1).toString());
                    }
                }
            }
        }

        // добавить чекбокс
        else if (r.field(i).type() == QVariant::UInt){
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
    QStringList query_str;

    while (!change_list.isEmpty()){
        QStringList args;

        auto *item = getRow(change_list.begin().key());

        for (auto &key : change_list.begin().value().keys()){
            args.append(header[key] + "=");

            switch (storage.first().at(key).type()) {
            case QVariant::Time:
            case QVariant::DateTime:
            case QVariant::String:
                args.last().append("'" + item->at(key).toString() + "'");
                break;
            default:
                args.last().append(item->at(key).toString());
                break;
            }
        }

        query_str.append("UPDATE " + currentTable + " SET " + args.join(", ") + " WHERE id=" + getRow(change_list.begin().key())->at(0).toString());

        change_list.remove(change_list.begin().key());
    }

    if (!query_str.isEmpty()){
        qDebug() << query_str.join("; ");

        QSqlQuery q(*database);

        if (!q.exec(query_str.join("; "))){
            QMessageBox msgBox;
            msgBox.setText("ERROR: " + QSqlError(database->lastError()).text());
            msgBox.exec();

            return false;
        }
    }

    return true;
}

void MyModel::reloadTable(){
    clean_isEnabled = true;
    loadTable(currentTable);
}

void MyModel::loadTable(QString TableName){
    if (TableName == "planes")
        select<essences::planes>();
    else if (TableName == "company")
        select<essences::company>();
    else if (TableName == "status")
        select<essences::status>();
    else if (TableName == "hangar")
        select<essences::hangar>();
    else if (TableName == "shedule")
        select<essences::shedule>();
    else if (TableName == "way")
        select<essences::way>();
    else{
        clear();
    }
}

bool MyModel::InsertData(){
    QStringList query_str;
    QRegularExpression re("ID$");

    for (;inserted_rows_count > 0; inserted_rows_count--){
        QStringList par;
        QStringList args;

        auto *item = getRow(storage.size() - inserted_rows_count);

        for (int key = 1; key < header.size(); key++){
            par.append(header[key]);

            switch (storage.first().at(key).type()) {
            case QVariant::Time:
            case QVariant::DateTime:
            case QVariant::String:
                args.append("'" + item->at(key).toString() + "'");
                break;
            case QVariant::Int:
                if (header[key].contains(re)){
                    if (item->at(key).toInt() == 0){
                        args.append("NULL");
                    } else {
                        args.append(item->at(key).toString());
                    }
                    break;
                }
            default:
                args.append(item->at(key).toString());
                break;
            }
        }

        query_str.append("INSERT INTO " + currentTable + " (" + par.join(", ") + ") VALUES (" + args.join(", ") + ")");
    }

    if (!query_str.isEmpty()){
        qDebug() << query_str.join("; ");
        QSqlQuery q(*database);

        if (!q.exec(query_str.join("; "))){
            QMessageBox msgBox;
            msgBox.setText("ERROR: " + QSqlError(database->lastError()).text());
            msgBox.exec();

            return false;
        }
    }

    return true;
}

bool MyModel::RemoveData(){
    QStringList query_str;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

    for (auto &index : selection){
        query_str.append("DELETE FROM " + currentTable + " WHERE id=" + getRow(index.row())->at(0).toString());
    }

    if (!query_str.isEmpty()){
        qDebug() << query_str.join("; ");
        QSqlQuery q(*database);

        if (!q.exec(query_str.join("; "))){
            QMessageBox msgBox;
            msgBox.setText("ERROR: " + QSqlError(database->lastError()).text());
            msgBox.exec();

            return false;
        }
    }

    return true;
}
