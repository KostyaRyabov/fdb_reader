#include "comboboxdelegate.h"
#include <QWidget>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QApplication>
#include <QDebug>
#include <QStyleOptionViewItemV4>

ComboBoxDelegate::ComboBoxDelegate(int column, MyModel *parent) : QItemDelegate(parent){
    keys = &parent->dictionaries[parent->header[column]].first;
    values = &parent->dictionaries[parent->header[column]].second;

    model = parent;
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    QComboBox *editor = new QComboBox(parent);

    editor->addItems(*values);
    auto selection = keys->indexOf(index.model()->data(index, Qt::DisplayRole).toInt());
    if (selection == -1) selection = 0;
    editor->setCurrentIndex(selection);

    return editor;
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, QVariant(keys->at(comboBox->currentIndex())), Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;

    auto selection = keys->indexOf(index.model()->data(index, Qt::DisplayRole).toInt());
    if (selection == -1) {
        selection = 0;
        model->setData(index, selection, Qt::EditRole);

        model->dataChanged(model->index(index.row(),0), model->index(index.row(), model->columnCount(index)-1), QVector<int>{Qt::BackgroundRole});
    }

    myOption.text = values->value(selection);
    myOption.backgroundBrush = QBrush(index.model()->data(index, Qt::BackgroundRole).value<QColor>());
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}
