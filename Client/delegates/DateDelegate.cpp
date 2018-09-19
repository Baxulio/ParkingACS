#include "DateDelegate.h"

DateDelegate::DateDelegate(QObject *parent):
    QItemDelegate (parent)
{

}

QWidget *DateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QDateEdit *editor = new QDateEdit(parent);
    editor->setCalendarPopup(true);
    return editor;
}

void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDate val = index.model()->data(index, Qt::EditRole).toDate();
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    dateEdit->setDate(val);
}

void DateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    QDate val = dateEdit->date();
    model->setData(index, val, Qt::EditRole);
}

void DateDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
