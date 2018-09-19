#ifndef ACCESSPARAMETERSFORM_H
#define ACCESSPARAMETERSFORM_H

#include <QWidget>

#include "DatabaseManager.h"
#include "models/CustomSqlTableModel.h"

#include "delegates/DateDelegate.h"
#include "delegates/AccessTypeDelegate.h"

namespace Ui {
class AccessParametersForm;
}

class AccessParametersForm : public QWidget
{
    Q_OBJECT

public:
    explicit AccessParametersForm(QWidget *parent = 0);
    ~AccessParametersForm();

private slots:
    void on_refresh();
    void on_submit_but_clicked();
    void on_revert_but_clicked();
    void on_add_price_but_clicked();
    void on_delete_price_but_clicked();
    void on_add_code_but_clicked();
    void on_delete_code_but_clicked();
    void on_price_table_clicked(const QModelIndex &index);

private:
    Ui::AccessParametersForm *ui;
    DatabaseManager &bDb;
    CustomSqlTableModel *priceModel, *codesModel;
    DateDelegate *dateDelegate;
    AccessTypeDelegate *accessTypeDelegate;
signals:
    void back();
};

#endif // ACCESSPARAMETERSFORM_H
