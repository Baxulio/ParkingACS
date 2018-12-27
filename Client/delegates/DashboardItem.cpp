#include "DashboardItem.h"
#include "ui_DashboardItem.h"

DashboardItem::DashboardItem(QString title, double cards_sum, double cars_sum, QWidget *parent):
    QFrame(parent),
    ui(new Ui::DashboardItem)
{
    ui->setupUi(this);
    ui->title->setText(title);
    ui->cards_sum_label->setText(QString::number(cards_sum,'f',2));
    ui->numbers_sum_label->setText(QString::number(cars_sum,'f',2));
    ui->total_sum_label->setText(QString::number(cards_sum+cars_sum,'f',2));
}

DashboardItem::~DashboardItem()
{
    delete ui;
}
