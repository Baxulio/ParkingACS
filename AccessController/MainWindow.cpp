#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bDb(DatabaseManager::instance()),
    bSettings(new SettingsDialog(this))
{
    ui->setupUi(this);

    connect(&bTimer, &QTimer::timeout, this, &MainWindow::timeout);
    readSettings();
    initActionsConnections();

    connect(&bDb,&DatabaseManager::connectionChanged, [this](bool status){
        if(status){
            ui->statusBar->showMessage("Successfully connected!");
        }
        else exit(EXIT_FAILURE);
    });

    ui->connectButton->click();
}

MainWindow::~MainWindow()
{
    delete bSettings;
    delete ui;
}

void MainWindow::wiegandCallback(quint32 code)
{
    qDebug()<<"fucka mucka: "<<code;
    if(bSettings->modeSettings().mode){
        if(!enter(code))return;

        digitalWrite(BAREER_PIN,HIGH);
        bTimer.start(bSettings->modeSettings().wait_time*1000);
        return;
    }

    if(!exit(code))return;

    digitalWrite(BAREER_PIN,HIGH);
    bTimer.start(bSettings->modeSettings().wait_time*1000);
    return;

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::initActionsConnections()
{
    connect(ui->configureButton, &QPushButton::clicked, bSettings, &SettingsDialog::show);
    connect(ui->aboutButton, &QPushButton::clicked, [this](){
        QMessageBox::about(this, tr("About Attraction"),
                           tr("The <b>Attraction</b> is an Access Controll System for amusement parks "
                              "with the following functionalities:"
                              "<ul><li>Maintain tariffs</li>"
                              "<li>Online deposit for visitors connected to their bracers</li>"
                              "<li>Selling any type of products</li>"
                              "<li>Work with Wiegand devices</li>"
                              "<li>Run on any platform</li>"
                              "<li>Controll the barrier<li>"
                              "<li>Print cheque with full transactions<li></ul>"
                              "<p align='right'>Author: <b>Nurnazarov Bakhmanyor Yunuszoda</b></p> <a align='right' href="
                              "'http://bahman.byethost18.com'>[bahman.byethost18.com]</a>"));
    });
    connect(ui->connectButton,&QPushButton::clicked, [this](){
        bDb.connectToDatabase(bSettings->serverSettings().host,
                              bSettings->serverSettings().user,
                              bSettings->serverSettings().password,
                              bSettings->serverSettings().port);
    });
    connect(ui->disconnectButton,&QPushButton::clicked, [this](){
        bDb.closeConnection();
    });
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }

    if(!settings.contains("server_host"))
        return;

    SettingsDialog::ServerSettings server;
    server.host = settings.value("server_host",QString()).toString();
    server.user = settings.value("server_user",QString()).toString();
    server.password = settings.value("server_password",QString()).toString();
    server.port = settings.value("server_port",quint32()).toUInt();

    bSettings->setServerSettings(server);

    SettingsDialog::ModeSettings mode;
    mode.bareerNumber = settings.value("bareer_number", quint8()).toUInt();
    mode.mode = settings.value("mode",bool()).toBool();
    mode.wait_time = settings.value("wait_time", quint32()).toUInt();

    bSettings->setMode(mode);
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());

    SettingsDialog::ServerSettings server = bSettings->serverSettings();
    settings.setValue("server_host", server.host);
    settings.setValue("server_user", server.user);
    settings.setValue("server_password", server.password);
    settings.setValue("server_port", server.port);

    SettingsDialog::ModeSettings mode = bSettings->modeSettings();
    settings.setValue("bareer_number",mode.bareerNumber);
    settings.setValue("mode",mode.mode);
    settings.setValue("wait_time",mode.wait_time);
}

bool MainWindow::enter(quint32 code)
{
    QSqlQuery query;
    if(!query.exec(QString("SELECT id, enter_time, tariff_id FROM active_bracers WHERE code=%1").arg(code))){
        bDb.debugQuery(query);
        return false;
    }

    query.next();
    if(!query.isValid()){
        ui->statusBar->showMessage("Карта не зарегистрирована!",2000);
        return false;
    }

    if(!query.value("enter_time").isNull()){
        ui->statusBar->showMessage("Уже зафиксирован при входе!",2000);
        return false;
    }
    enterRec = query.record();

    return true;
}

bool MainWindow::exit(quint32 code)
{
    QSqlQuery query;
    if(!query.exec(QString("SELECT *, SYSDATE() AS cur_date_time "
                           "FROM ((active_bracers "
                           "INNER JOIN deposit ON deposit.id=deposit_id) "
                           "INNER JOIN tariff ON tariff.id=tariff_id) "
                           "WHERE code=%1 AND NOT ISNULL(enter_time)").arg(code))){
        bDb.debugQuery(query);
        return false;
    }

    query.next();

    if(!query.isValid()){
        ui->statusBar->showMessage("Карта не зарегистрирована!",2000);
        return false;
    }

    QDateTime enterTime = query.value("enter_time").toDateTime();

    quint64 timeLimit= enterTime.secsTo(query.value("expected_exit_time").toDateTime())/60;
    quint64 elapsed = enterTime.secsTo(query.value("cur_date_time").toDateTime())/60;

    if(elapsed>timeLimit){
        //here goes penalty
        double cache = query.value("cash").toDouble();

        int minutes = int(elapsed%60==0?elapsed/60:1+elapsed/60)*60;

        cache-=minutes*(query.value("price").toDouble()/timeLimit);

        if(!query.exec(QString("CALL penalty(%1,%2,%3,%4);")
                       .arg(minutes)
                       .arg(query.value("id").toInt())
                       .arg(cache)
                       .arg(query.value("deposit_id").toInt()))){
            bDb.debugQuery(query);
            return false;
        }
        if(!query.exec(QString("SELECT *, SYSDATE() AS cur_date_time "
                               "FROM ((active_bracers "
                               "INNER JOIN deposit ON deposit.id=deposit_id) "
                               "INNER JOIN tariff ON tariff.id=tariff_id) "
                               "WHERE code=%1 AND NOT ISNULL(enter_time)").arg(code))){
            bDb.debugQuery(query);
            return false;
        }
    }

    double cache = query.value("cash").toDouble();

    if(cache==0){
        print("",query.record());
        return true;
    }
    else if(cache<0){
        print("Долг",query.record());
        return false;
    }
    else {
        print("Остаток",query.record());
        return false;
    }
}

void MainWindow::print(const QString &title, const QSqlRecord &record)
{
    char t[1000];
    sprintf(t,"printf '********************************\n"
              "OASIS\n\n"
              "Braslet: %d\n"
              "Vxod: %s    [%02d]\n"
              "Vixod: %s   [%02d]\n"
              "Tarif: %s\n"
              "Balans: .2f\n"
              "Transaktsii:\n"
              "-------------------\n"
              ""
              "-------------------\n"
              "********** powered by GSS.UZ\n\n\n\n' >/dev/usb/lp0",
            record.value("bracer_number").toUInt(),
            record.value("enter_time").toDateTime().toString("dd.MM.yyyy H:mm").toLatin1().data(),
            record.value("enter_number").toInt(),
            record.value("cur_date_time").toDateTime().toString("dd.MM.yyyy H:mm").toLatin1().data(),
            bSettings->modeSettings().bareerNumber,
            record.value("title").toString(),
            record.value("cash").toDouble()
            );
    system(t);
}

void MainWindow::interrupt()
{
    QSqlQuery query;
    if(bSettings->modeSettings().mode and !query.exec(QString("UPDATE active_bracers "
                                                              "SET enter_time=SYSDATE(), "
                                                              "enter_number=%1, "
                                                              "expected_exit_time=SYSDATE() + INTERVAL (SELECT time_limit FROM tariff WHERE id=%2) MINUTE "
                                                              "WHERE id=%3")
                                                      .arg(bSettings->modeSettings().bareerNumber)
                                                      .arg(enterRec.value("tariff_id").toInt())
                                                      .arg(enterRec.value("id").toInt()))){
        bDb.debugQuery(query);
        return;
    }

    if(!bSettings->modeSettings().mode and !query.exec(QString("CALL move_to_history(%1,%2)")
                                                       .arg(bSettings->modeSettings().bareerNumber)
                                                       .arg(exitRec.value("id").toInt()))){
        bDb.debugQuery(query);
        return;
    }
    timeout();
}

void MainWindow::timeout()
{
    digitalWrite(BAREER_PIN,LOW);
    bTimer.stop();
    enterRec.clearValues();
    exitRec.clearValues();
}
