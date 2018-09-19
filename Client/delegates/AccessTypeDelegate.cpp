#include "AccessTypeDelegate.h"

#include <QComboBox>

AccessTypeDelegate::AccessTypeDelegate(QObject *parent):
    QItemDelegate(parent)
{
    accessTypes<<"Оплата"<<"Допуск"<<"Запрет";
}


QWidget *AccessTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QComboBox *editor = new QComboBox(parent);
    editor->addItems(accessTypes);
    return editor;
}

void AccessTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString val = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboEdit = static_cast<QComboBox*>(editor);
    comboEdit->setCurrentText(val);
}

void AccessTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboEdit = static_cast<QComboBox*>(editor);
    QString val = comboEdit->currentText();
    model->setData(index, val, Qt::EditRole);
}

void AccessTypeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
