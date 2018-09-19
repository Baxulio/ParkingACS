#include "CustomSqlTableModel.h"
#include <QColor>
#include <QBrush>

CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{
}

QVariant CustomSqlTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole && isDirty(index))
        return QBrush(QColor(Qt::yellow));
    return QSqlTableModel::data(index, role);
}
