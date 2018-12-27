#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include "delegates/FlowLayout.h"

#include "dialogs/SettingsDialog.h"
#include "DatabaseManager.h"

namespace Ui {
class Dashboard;
}

class Dashboard : public QWidget
{
    Q_OBJECT

public:
    explicit Dashboard(SettingsDialog &set, QWidget *parent = 0);
    ~Dashboard();

private slots:
    void on_refresh();

private:
    Ui::Dashboard *ui;
    FlowLayout *layout;
    DatabaseManager &bDb;
    SettingsDialog &bSettings;

    double cards_total, numbers_total;


private:
    void fetchData(QString url);
signals:
    void message(const QString &text);
};

#endif // DASHBOARD_H
