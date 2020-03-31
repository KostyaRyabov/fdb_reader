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

        bool select(QString tableName);
};


#endif // MYMODEL_H
