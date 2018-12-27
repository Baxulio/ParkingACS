#include "Dashboard.h"
#include "ui_Dashboard.h"

#include "delegates/DashboardItem.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>

Dashboard::Dashboard(SettingsDialog &set, QWidget *parent):
    QWidget(parent),
    ui(new Ui::Dashboard),
    bDb(DatabaseManager::instance()),
    layout(new FlowLayout(0,10,10)),
    bSettings(set)
{
    ui->setupUi(this);

    connect(&bDb, &DatabaseManager::refresh, this, &Dashboard::on_refresh);

    ui->frame->setLayout(layout);
    ui->today_label->setText(QDate::currentDate().toString());
}

Dashboard::~Dashboard()
{
    delete ui;
}

void Dashboard::on_refresh()
{
    if(!this->isVisible())return;

    cards_total = 0;
    numbers_total = 0;

    QLayoutItem *it;
    while((it = layout->takeAt(0))){
        it->widget()->setParent(NULL);
        delete it;
    }

    foreach (QString url, bSettings.urls()) {
        fetchData(url);
    }

    ui->cards_sum_label->setText(QString::number(cards_total,'f',2));
    ui->numbers_sum_label->setText(QString::number(numbers_total,'f',2));
    ui->total_sum_label->setText(QString::number(cards_total+numbers_total,'f',2));
}

void Dashboard::fetchData(QString url)
{
    QStringList strl = url.split(":",QString::SkipEmptyParts);
    if(strl.size()<2)return;

    QString address = strl.last();
    address.remove(' ');

    QSqlDatabase db(QSqlDatabase::addDatabase("QMYSQL",address));
    db.setDatabaseName("parking_acs");
    db.setHostName(address);
    db.setPort(bSettings.serverSettings().port);
    if(!db.open(bSettings.serverSettings().user,bSettings.serverSettings().password)){
        db.removeDatabase(address);
        return;
    }

    QSqlQuery query(db), sub_query(db);

    if(!query.exec("SELECT DISTINCT out_number FROM History WHERE DATE(out_time)=CURDATE() ORDER BY out_number")){
        db.close();
        db.removeDatabase(address);
        return ;
    }

    while(query.next()){
        int out_number = query.value("out_number").toInt();
        sub_query.exec(QString("SELECT SUM(price) AS total_price FROM History WHERE DATE(out_time)=CURDATE() AND out_number=%1 AND code_type = 'Карта'").arg(out_number));
        sub_query.next();
        double cards_sum = sub_query.value("total_price").toDouble();

        sub_query.exec(QString("SELECT SUM(price) AS total_price FROM History WHERE DATE(out_time)=CURDATE() AND out_number=%1 AND code_type = 'Номер'").arg(out_number));
        sub_query.next();
        double numbers_sum = sub_query.value("total_price").toDouble();

        cards_total+=cards_sum;
        numbers_total+=numbers_sum;

        layout->addWidget(new DashboardItem(QString("%1 %3: %2").arg(strl.first()).arg(address).arg(out_number),cards_sum,numbers_sum));
    }
    db.close();
    db.removeDatabase(url);
}
