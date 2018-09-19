#ifndef CUSTOMSQLTABLEMODEL_H
#define CUSTOMSQLTABLEMODEL_H

#include "core_global.h"
#include <QSqlTableModel>

class CustomSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit CustomSqlTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());


    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
};

#endif // CUSTOMSQLTABLEMODEL_H
