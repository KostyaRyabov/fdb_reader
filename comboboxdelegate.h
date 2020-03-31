#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <mymodel.h>
#include <QItemDelegate>
#include <QComboBox>

class MyModel;

class ComboBoxDelegate : public QItemDelegate
{
public:
    ComboBoxDelegate(int column, MyModel *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QList<int> *keys;
    QStringList *values;
};

#endif // COMBOBOXDELEGATE_H
