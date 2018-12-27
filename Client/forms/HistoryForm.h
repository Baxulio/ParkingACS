#ifndef HISTORYFORM_H
#define HISTORYFORM_H

#include <QWidget>

#include "DatabaseManager.h"
#include "models/CurrentProxyModel.h"
#include "models/CustomSqlTableModel.h"

#include <QNetworkAccessManager>
#include "dialogs/SettingsDialog.h"

namespace Ui {
class HistoryForm;
}

class HistoryForm : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryForm(SettingsDialog &set, QWidget *parent = 0);
    ~HistoryForm();

private slots:
    void on_refresh();
    void on_in_today_but_clicked();
    void on_clean_but_clicked();
    void on_filter_but_clicked();
    void on_filter_but_triggered(QAction *arg1);
    void on_back_but_clicked();
    void on_out_today_but_clicked();

    void on_print_but_clicked();

    void on_csv_but_clicked();

private:
    Ui::HistoryForm *ui;
    SettingsDialog &bSettings;
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

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // HISTORYFORM_H
