#ifndef DASHBOARDITEM_H
#define DASHBOARDITEM_H

#include <QFrame>

namespace Ui {
class DashboardItem;
}

class DashboardItem : public QFrame
{
    Q_OBJECT

public:
    explicit DashboardItem(QString title, double cards_sum, double cars_sum, QWidget *parent = 0);
    ~DashboardItem();

private:
    Ui::DashboardItem *ui;
};

#endif // DASHBOARDITEM_H
