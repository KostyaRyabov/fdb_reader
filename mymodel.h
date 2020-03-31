#ifndef MYMODEL_H
#define MYMODEL_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAbstractTableModel>

#include <comboboxdelegate.h>
#include <checkboxdelegate.h>

#include <QObject>
#include <QSqlRecord>
#include <QLinkedList>
#include "essences.h"
#include <QTableView>
#include <QSqlQuery>
#include <QMap>
#include <QHash>
#include <QVector>
#include <QList>
#include <QMessageBox>
#include "QSqlError"
#include <QDebug>
#include <QComboBox>
#include <QVariant>

#include <QSqlField>

class MyModel: public QAbstractTableModel
{
        Q_OBJECT
    public:
        MyModel(QObject* parent = 0 );
        ~MyModel();

        int rowCount( const QModelIndex& parent ) const;
        int columnCount( const QModelIndex& parent ) const;
        QVariant data( const QModelIndex& index, int role) const;
        bool setData( const QModelIndex& index, const QVariant& value, int role);
        QVariant headerData( int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags( const QModelIndex& index ) const;

        void getHeader(QSqlRecord &r);
        void clear();

        void bindView(Ui::MainWindow *ui);
        void bindDataBase(QSqlDatabase &db);

        bool UpdateData();
        bool InsertData();
        bool RemoveData();

        void AddEmptyRow();
        void reloadTable();
        void loadTable(QString TableName);

        bool saveData();
        bool loadData();
    private:
        bool clean_isEnabled = true;
        int inserted_rows_count = 0;

        QString currentTable;

        QSqlDatabase *database;

        QLinkedList<essences::o> storage;
        QHash<QString, QPair<QList<int>, QStringList>> dictionaries;
        QHash<int, QHash<int, QVariant>> change_list;

        QStringList header;
        essences::o* getRow(int i) const;

        Ui::MainWindow *ui;
        QList<QAbstractItemDelegate*> delegateForColumn;

        friend class ComboBoxDelegate;
signals:
        void setTitle(QString newValue);
    public:
        friend QDataStream &operator<<(QDataStream &out, MyModel &model);
        friend QDataStream &operator>>(QDataStream &in, MyModel &model);

        template <typename T>
        bool select(){
            QSqlQuery q(*database);

            if (clean_isEnabled){
                clear();
            }

            currentTable = QString::fromUtf8(T::getName());
            q.prepare("SELECT * from " + currentTable);

            if (!q.exec()){
                QMessageBox msgBox;
                msgBox.setText("ERROR: " + QSqlError(database->lastError()).text());
                msgBox.exec();
                return 0;
            }

            if (clean_isEnabled){
                QSqlRecord r = q.record();
                getHeader(r);

                emit setTitle(currentTable);
            } else {
                clean_isEnabled = true;
            }

            int i = 0, row;
            while (q.next()){
                row = storage.count();
                beginInsertRows( QModelIndex(), row, row );

                T item;

                for (i = 0; i < item.size(); i++) {
                    switch (item.at(i).type()) {
                    case QVariant::Time:
                        item.at(i) = q.value(i).toTime();
                        break;
                    default:
                        item.at(i) = q.value(i);
                        break;
                    }
                }

                storage << item;
                endInsertRows();
            }

            if (storage.size() > 0)
                ui->add_bn->setEnabled(true);

            ui->save_bn->setEnabled(true);

            return 1;
        }
};


#endif // MYMODEL_H
