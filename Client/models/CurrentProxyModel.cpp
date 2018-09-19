#include "CurrentProxyModel.h"
#include <QSqlTableModel>

CurrentProxyModel::CurrentProxyModel(QObject *par):
    QSortFilterProxyModel(par),
    in(0),out(0),code(""),code_card_type(true),code_plate_type(true),
    access_allowed_type(true),access_denied_type(true),access_charged_type(true)
{}

bool CurrentProxyModel::dateInRange(const QDateTime &dateTime, bool mode) const
{
    if(mode){
        return (!in_time_from.isValid() || dateTime>=in_time_from)
                && (!in_time_to.isValid() || dateTime<=in_time_to);
    }
    return (!out_time_from.isValid() || dateTime>=out_time_from)
            && (!out_time_to.isValid() || dateTime<=out_time_to);

}

bool CurrentProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(in && sourceModel()->data(sourceModel()->index(source_row, static_cast<QSqlTableModel*>(sourceModel())->fieldIndex("in_number"), source_parent)).toInt()!=in)
        return false;
    if(out && sourceModel()->data(sourceModel()->index(source_row, static_cast<QSqlTableModel*>(sourceModel())->fieldIndex("out_number"), source_parent)).toInt()!=out)
        return false;

    if(code!="" && !sourceModel()->data(sourceModel()->index(source_row, static_cast<QSqlTableModel*>(sourceModel())->fieldIndex("code"), source_parent)).toString().startsWith(code))
        return false;

    QString temp = sourceModel()->data(sourceModel()->index(source_row, static_cast<QSqlTableModel*>(sourceModel())->fieldIndex("code_type"), source_parent)).toString();
    if(!code_card_type && temp=="Карта")
        return false;
    if(!code_plate_type && temp=="Номер")
        return false;

    temp = sourceModel()->data(sourceModel()->index(source_row, static_cast<QSqlTableModel*>(sourceModel())->fieldIndex("access_type"), source_parent)).toString();
    if(!access_allowed_type && temp=="Допуск")
        return false;
    if(!access_denied_type && temp=="Запрет")
        return false;
    if(!access_charged_type && temp=="Оплата")
        return false;

    if(!dateInRange(sourceModel()->data(sourceModel()->index(source_row,  static_cast<QSqlTableModel*>(sourceModel())->fieldIndex("in_time"), source_parent)).toDateTime(),true))
        return false;

    if(!dateInRange(sourceModel()->data(sourceModel()->index(source_row,  static_cast<QSqlTableModel*>(sourceModel())->fieldIndex("out_time"), source_parent)).toDateTime(),false))
        return false;

    return true;
}
