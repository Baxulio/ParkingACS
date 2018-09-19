#ifndef CURRENTFORM_H
#define CURRENTFORM_H

#include <QWidget>

#include "DatabaseManager.h"
#include "models/CurrentProxyModel.h"
#include "models/CustomSqlTableModel.h"

#include <QNetworkAccessManager>

namespace Ui {
class CurrentForm;
}

class CurrentForm : public QWidget
{
    Q_OBJECT

public:
    explicit CurrentForm(QWidget *parent = 0);
    ~CurrentForm();

private slots:
    void on_refresh();
    void on_in_today_but_clicked();
    void on_clean_but_clicked();
    void on_filter_but_clicked();
    void on_filter_but_triggered(QAction *arg1);

    void on_back_but_clicked();

    void on_print_but_clicked();

    void on_csv_but_clicked();

private:
    Ui::CurrentForm *ui;
    QNetworkAccessManager networkManager;

    DatabaseManager &bDb;
    CustomSqlTableModel *model;
    CurrentProxyModel *proxyModel;


private:
    void updateInfo();
    void clearFields();
    void syncFieldsWithProxy();

signals:
    void back();
    void message(const QString &text);
};

#endif // CURRENTFORM_H
