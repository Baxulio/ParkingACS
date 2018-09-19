#ifndef DATEDELEGATE_H
#define DATEDELEGATE_H

#include <QItemDelegate>
#include <QDateEdit>

class DateDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DateDelegate(QObject *parent = 0);

    // QAbstractItemDelegate interface
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // DATEDELEGATE_H
