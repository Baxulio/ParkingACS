#include "AccessParametersForm.h"
#include "ui_AccessParametersForm.h"

#include <QSqlRecord>
#include<QDebug>

AccessParametersForm::AccessParametersForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccessParametersForm),
    bDb(DatabaseManager::instance()),
    priceModel(new CustomSqlTableModel(this)),
    codesModel(new CustomSqlTableModel(this)),
    dateDelegate(new DateDelegate(this)),
    accessTypeDelegate(new AccessTypeDelegate(this))
{
    ui->setupUi(this);

    priceModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    codesModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->price_table->setModel(priceModel);
    ui->code_table->setModel(codesModel);

    connect(&bDb, &DatabaseManager::refresh, this, &AccessParametersForm::on_refresh);
    connect(ui->price_table->selectionModel(), &QItemSelectionModel::currentRowChanged, [this](const QModelIndex &cur, const QModelIndex &prev){
        Q_UNUSED(prev)
        on_price_table_clicked(cur);
    });
    connect(priceModel, &CustomSqlTableModel::dataChanged, [this]{
        ui->submit_but->setEnabled(true);
        ui->revert_but->setEnabled(true);
    });
    connect(codesModel, &CustomSqlTableModel::dataChanged, [this]{
        ui->submit_but->setEnabled(true);
        ui->revert_but->setEnabled(true);
    });
}

AccessParametersForm::~AccessParametersForm()
{
    delete ui;
}

void AccessParametersForm::on_refresh()
{
    if(!this->isVisible())return;

    ui->submit_but->setEnabled(false);
    ui->revert_but-> setEnabled(false);

    QModelIndex indx;
    bool b=ui->price_table->selectionModel()->hasSelection();
    if(b){
        indx = ui->price_table->selectionModel()->selectedIndexes().first();
    }

    priceModel->setTable("Price");

    if(!priceModel->select()){
        bDb.debugError(priceModel->lastError());
        return ;
    }
    ui->price_table->hideColumn(priceModel->fieldIndex("id"));
    priceModel->setHeaderData(priceModel->fieldIndex("car_type"), Qt::Horizontal, "Тип", Qt::DisplayRole);
    priceModel->setHeaderData(priceModel->fieldIndex("price_formula"), Qt::Horizontal, "Формула цены", Qt::DisplayRole);

    codesModel->setTable("Reserved_codes");

    codesModel->setHeaderData(codesModel->fieldIndex("code"), Qt::Horizontal, "Код [н/к]", Qt::DisplayRole);
    codesModel->setHeaderData(codesModel->fieldIndex("access_type"), Qt::Horizontal, "Тип доступа", Qt::DisplayRole);
    codesModel->setHeaderData(codesModel->fieldIndex("starting_date"), Qt::Horizontal, "Начиная с", Qt::DisplayRole);
    codesModel->setHeaderData(codesModel->fieldIndex("duration"), Qt::Horizontal, "Продолжительность (дней)", Qt::DisplayRole);

    ui->price_table->setCurrentIndex(b?indx:priceModel->index(0,0));

    ui->code_table->setItemDelegateForColumn(codesModel->fieldIndex("starting_date"),dateDelegate);
    ui->code_table->setItemDelegateForColumn(codesModel->fieldIndex("access_type"),accessTypeDelegate);
}

void AccessParametersForm::on_submit_but_clicked()
{
    if(!codesModel->submitAll()){
        bDb.debugError(codesModel->lastError());
        return;
    }
    QModelIndex curIndx = ui->price_table->currentIndex();
    if(!priceModel->submitAll()){
        bDb.debugError(codesModel->lastError());
        return;
    }
    ui->price_table->setCurrentIndex(curIndx);
    ui->submit_but->setEnabled(false);
    ui->revert_but->setEnabled(false);
}

void AccessParametersForm::on_revert_but_clicked()
{
    on_refresh();
}

void AccessParametersForm::on_add_price_but_clicked()
{
    if(!priceModel)
        return;

    QModelIndex insertIndex = ui->price_table->currentIndex();
    int row = insertIndex.row() == -1 ? 0 : insertIndex.row();
    priceModel->insertRow(row);
    insertIndex = priceModel->index(row, priceModel->fieldIndex("car_type"));
    ui->price_table->setCurrentIndex(insertIndex);
    ui->price_table->edit(insertIndex);
}

void AccessParametersForm::on_delete_price_but_clicked()
{
    if(!priceModel)
        return;

    QModelIndexList currentSelection = ui->price_table->selectionModel()->selectedRows(priceModel->fieldIndex("car_type"));
    for (int i = 0; i < currentSelection.count(); ++i) {
        QString temp = priceModel->data(currentSelection.at(i),Qt::EditRole).toString();
        if (temp != "Другое" and temp != "Черный список" and temp != "Белый список"){
            priceModel->removeRow(currentSelection.at(i).row());
            ui->submit_but->setEnabled(true);
            ui->revert_but->setEnabled(true);
        }
    }
}

void AccessParametersForm::on_add_code_but_clicked()
{
    if(!codesModel or !ui->code_table->isEnabled())
        return;

    QModelIndex insertIndex = ui->code_table->currentIndex();
    int row = insertIndex.row() == -1 ? 0 : insertIndex.row();

    QSqlRecord rec = priceModel->record(ui->price_table->currentIndex().row());

    codesModel->insertRow(row);

    insertIndex = codesModel->index(row, codesModel->fieldIndex("starting_date"));
    codesModel->setData(insertIndex,QDate::currentDate());

    if(rec.value("car_type").toString()=="Черный список"){
        insertIndex = codesModel->index(row, codesModel->fieldIndex("duration"));
        codesModel->setData(insertIndex,0);

        insertIndex = codesModel->index(row, codesModel->fieldIndex("access_type"));
        codesModel->setData(insertIndex,"Запрет");
    }
    if(rec.value("car_type").toString()=="Белый список"){
        insertIndex = codesModel->index(row, codesModel->fieldIndex("duration"));
        codesModel->setData(insertIndex,0);

        insertIndex = codesModel->index(row, codesModel->fieldIndex("access_type"));
        codesModel->setData(insertIndex,"Допуск");
    }

    insertIndex = codesModel->index(row, codesModel->fieldIndex("price_id"));
    codesModel->setData(insertIndex,rec.value("id").toInt());

    insertIndex = codesModel->index(row, codesModel->fieldIndex("code"));
    ui->code_table->setCurrentIndex(insertIndex);
    ui->code_table->edit(insertIndex);
}

void AccessParametersForm::on_delete_code_but_clicked()
{
    if(!codesModel or !ui->code_table->isEnabled())
        return;
    QModelIndexList currentSelection = ui->code_table->selectionModel()->selectedRows();
    for (int i = 0; i < currentSelection.count(); ++i) {
        codesModel->removeRow(currentSelection.at(i).row());
        ui->submit_but->setEnabled(true);
        ui->revert_but->setEnabled(true);
    }
}

void AccessParametersForm::on_price_table_clicked(const QModelIndex &index)
{
    if(priceModel->data(priceModel->index(index.row(),priceModel->fieldIndex("id")),Qt::EditRole).isNull() or
            priceModel->data(priceModel->index(index.row(),priceModel->fieldIndex("car_type")),Qt::EditRole).toString()=="Другое"){
        ui->code_table->setEnabled(false);
        return;
    }
    ui->code_table->setEnabled(true);

    codesModel->setFilter(QString("price_id=%1").arg(priceModel->data(priceModel->index(index.row(),priceModel->fieldIndex("id")),Qt::EditRole).toInt()));
    if(!codesModel->select()){
        bDb.debugError(codesModel->lastError());
        return ;
    }
    ui->code_table->hideColumn(codesModel->fieldIndex("id"));
    ui->code_table->hideColumn(codesModel->fieldIndex("price_id"));
}

void AccessParametersForm::on_search_but_clicked()
{
    if(ui->code_table->isEnabled()){
        for(int i=0; i<codesModel->rowCount(); i++){
            if(codesModel->index(i,codesModel->fieldIndex("code")).data(Qt::EditRole).toString().startsWith(ui->search_lineEdit->text(),Qt::CaseInsensitive)){
                ui->code_table->selectRow(i); return;
            }
        }
    }
}
