#ifndef ACCESSTYPEDELEGATE_H
#define ACCESSTYPEDELEGATE_H

#include <QItemDelegate>
#include <QComboBox>

class AccessTypeDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    AccessTypeDelegate(QObject *parent = 0);

private:
    QStringList accessTypes;

    // QAbstractItemDelegate interface
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // ACCESSTYPEDELEGATE_H
