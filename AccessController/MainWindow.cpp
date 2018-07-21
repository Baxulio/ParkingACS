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
    bSettings(new SettingsDialog(this)),
    openalpr("eu")
{
    ui->setupUi(this);

    openalpr.setDefaultRegion("uz");
//        openalpr.setTopN(15);

    if(!openalpr.isLoaded()){
        exit(EXIT_FAILURE);
    }

    readSettings();
    initActionsConnections();

    //    manager.connectToHost(bSettings->modeSettings().cameraIP,34567);
    request.setUrl(QUrl("http://192.168.0.238/webcapture.jpg?command=snap&channel=1"));

    connect(&bDb,&DatabaseManager::connectionChanged, [this](bool status){
        if(status){
            ui->statusBar->showMessage("Successfully connected!");
            qDebug()<<"request";
            manager.get(request);
            qDebug()<<"success";
        }
        else exit(EXIT_FAILURE);
    });

    movie.setFileName(":/resources/information.gif");
    bTimer.setInterval(5000);

    connect(&bTimer, &QTimer::timeout, [this]{
        ui->label->setMovie(&movie);
        movie.start();

        ui->enter_time_label->setText("");
        ui->enter_bareer_label->setText("");
        ui->plate_number_label->setText("Ожидание!");

        if(!bSettings->modeSettings().mode){
            ui->exit_time_label->setText("");
            ui->exit_bareer_label->setText("");
        }
    });

    connect(&manager, &QNetworkAccessManager::finished, this, &MainWindow::handleReply);
//    ui->connectButton->click();
}

MainWindow::~MainWindow()
{
    delete bSettings;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::print(const QString &code, const QString &dur, double price, const QDateTime &in_time, const QDateTime &out_time, const quint32 in)
{
    char t[1000];
    sprintf(t,"printf '********************************\nTashkent Trade Center [%02d]\n\nKod: %d\nKirish vaqti: %s\nChiqish vaqti: %s\nTurgan vaqti: %s\nPul miqdori: %.2f\nKirish #: %d\n********** powered by GSS.UZ\n\n\n\n' >/dev/usb/lp0",
            bSettings->modeSettings().bareerNumber,
            code.toLatin1().data(),
            in_time.toString("dd.MM.yyyy H:mm").toLatin1().data(),
            out_time.toString("dd.MM.yyyy H:mm").toLatin1().data(),
            dur.toLatin1().data(),
            price,
            in);
    system(t);
}

void MainWindow::proceedCode(QString code, bool plateMode)
{
    movie.stop();
    bTimer.stop();

    if(!plateMode){
        disconnect(&manager, &QNetworkAccessManager::finished, NULL, NULL);
    }

    QSqlQuery query;


    if(!query.exec(QString("SELECT SYSDATE();"))){
        bDb.debugQuery(query);
        return;
    }

    query.next();

    QDateTime b_out_time = query.value("SYSDATE()").toDateTime();

    if(!query.exec(QString("SELECT id, in_time,in_number "
                           "FROM Active WHERE code='%1';").arg(code))){
        bDb.debugQuery(query);
        return;
    }
    query.next();

    //ENTER
    if(bSettings->modeSettings().mode)
    {
        if(query.isValid()){

            ui->label->setText(QString("<p align=\"center\"><span style=\"font-size:100pt; color:#005500;\">%1</span></p>")
                               .arg(code));
            ui->plate_number_label->setText("Уже зарегистрирован!");
            ui->enter_time_label->setText(query.value("in_time").toDateTime().toString("dd.MM.yyyy H:mm"));
            ui->enter_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

            if(plateMode)openBareer();

            bTimer.start();

            if(!plateMode){
                connect(&manager, &QNetworkAccessManager::finished, this, &MainWindow::handleReply);
                manager.get(request);
            }

            return;
        }
    qDebug()<<code<<b_out_time;
        query.prepare("INSERT INTO Active(`code`, `in_time`, `in_number`, `img`) "
                      "VALUES(?,?,?,?);");
        query.addBindValue(code);
        query.addBindValue(b_out_time);
        query.addBindValue(bSettings->modeSettings().bareerNumber);
        query.addBindValue(plateMode?QString(currentFrame):"");

        if(!query.exec()){
            bDb.debugQuery(query);
            return;
        }

        ui->label->setText(QString("<p align=\"center\"><span style=\"font-size:100pt; color:#005500;\">%1</span></p>")
                           .arg(code));
        ui->plate_number_label->setText("Успешно!");
        ui->enter_time_label->setText(b_out_time.toString("dd.MM.yyyy H:mm"));
        ui->enter_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

        openBareer();

        bTimer.start();

        if(!plateMode){
            connect(&manager, &QNetworkAccessManager::finished, this, &MainWindow::handleReply);
            manager.get(request);
        }
        return;
    }

    //EXIT
    if(!query.isValid()){
        if(!query.exec(QString("SELECT in_time, in_number, out_time, price "
                               "FROM History "
                               "WHERE out_time >= DATE_SUB(NOW() , INTERVAL 2 MINUTE) "
                               "AND code=%1 LIMIT 1")
                       .arg(code))){
            bDb.debugQuery(query);
            return;
        }

        query.next();

        if(!query.isValid()){
            ui->label->setText(QString("<p align=\"center\"><span style=\"font-size:100pt; color:#ff0000;\">%1</span></p>")
                               .arg(code));
            ui->plate_number_label->setText("Не зафиксирован при въезде!");
            ui->enter_time_label->setText("");
            ui->enter_bareer_label->setText("");
            ui->exit_time_label->setText("");
            ui->exit_bareer_label->setText("");

            bTimer.start();
            return;
        }

        QDateTime in_time = query.value("in_time").toDateTime();
        QDateTime out_time = query.value("out_time").toDateTime();

        int diff = in_time.time().secsTo(out_time.time());
        int hours = diff/3600;
        int minutes = (diff%3600)/60;
        int secs = (diff%3600)%60;
        QTime time(hours,minutes,secs);

        ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                                   "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2 </span><span style=\" font-size:48pt; color:#ea0003;\">(UZS)</span></p>")
                           .arg(time.toString()).arg(query.value("price").toString()));


        ui->enter_time_label->setText(in_time.toString("dd.MM.yyyy H:mm"));
        ui->enter_bareer_label->setText(query.value("in_number").toString());
        ui->exit_time_label->setText(out_time.toString("dd.MM.yyyy H:mm"));
        ui->exit_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

        ui->plate_number_label->setText("Код уже деактивирован!");

        openBareer();
        bTimer.start();

        if(!plateMode){
            connect(&manager, &QNetworkAccessManager::finished, this, &MainWindow::handleReply);
            manager.get(request);
        }
        return;
    }

    int b_id = query.value("id").toInt();
    QDateTime b_in_time = query.value("in_time").toDateTime();
    quint32  b_in_number = query.value("in_number").toUInt();

    //    if(!query.exec(QString("SELECT Price.price_formula, Cards.subscription "
    //                           "FROM Price INNER JOIN Cards ON Price.id = Cards.priceId "
    //                           "WHERE Cards.code = %1;")
    //                   .arg(code))){
    //        bDb.debugQuery(query);
    //        return;
    //    }

    //    query.next();

    //    if(!query.isValid()){

    if(!query.exec("SELECT price_formula FROM Price WHERE car_type = 'Другое'")){
        bDb.debugQuery(query);
        return;
    }
    query.next();

    //    }
    //    else if(query.value("subscription").toBool()){
    //        emit Result(Replies::WIEGAND_IS_MONTHLY,b_in_time,b_in_number,b_out_time);
    //        return;
    //    }



    double price = calculate_formula(query.value("price_formula").toString(),
                                     b_in_time.secsTo(b_out_time));

    qDebug()<<b_id<<price;

    if(!query.exec(QString("CALL MoveToHistory('%1','%2','%3','%4');")
                   .arg(b_id)
                   .arg(price)
                   .arg(bSettings->modeSettings().bareerNumber)
                   .arg(plateMode?QString(currentFrame):""))){
        bDb.debugQuery(query);
        return;
    }

    int diff = b_in_time.time().secsTo(b_out_time.time());
    int hours = diff/3600;
    int minutes = (diff%3600)/60;
    int secs = (diff%3600)%60;
    QTime time(hours,minutes,secs);

    ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                               "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2 </span><span style=\" font-size:48pt; color:#ea0003;\">(UZS)</span></p>")
                       .arg(time.toString()).arg(price));


    ui->enter_time_label->setText(b_in_time.toString("dd.MM.yyyy H:mm"));
    ui->enter_bareer_label->setText(QString::number(b_in_number));
    ui->exit_time_label->setText(b_out_time.toString("dd.MM.yyyy H:mm"));
    ui->exit_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

    ui->plate_number_label->setText("Успешно деактивирован!");

    print(code,time.toString(),price,b_in_time, b_out_time, b_in_number);
    openBareer();
    bTimer.start();

    if(!plateMode){
        connect(&manager, &QNetworkAccessManager::finished, this, &MainWindow::handleReply);
        manager.get(request);
    }

    return;
}

void MainWindow::handleReply(QNetworkReply *pReply)
{
    qDebug()<<"Starting reading reply";
    currentFrame=pReply->readAll();
    std::vector<char> vector(currentFrame.begin(),currentFrame.end());
    results=openalpr.recognize(vector);

    qDebug()<<"plates = "<<results.plates.size();

    if(results.plates.size())
        proceedCode(QString::fromStdString(results.plates[0].bestPlate.characters),true);

    manager.get(request);
}

void MainWindow::wiegandCallback(quint32 value)
{
    proceedCode(QString::number(value),false);
}

void MainWindow::showStatusMessage(const QString &message)
{

}

void MainWindow::initActionsConnections()
{
    connect(ui->configureButton, &QPushButton::clicked, bSettings, &SettingsDialog::show);
    connect(ui->aboutButton, &QPushButton::clicked, [this](){
        QMessageBox::about(this, tr("About ParkingACS"),
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
    mode.cameraIP = settings.value("cameraIP", QString()).toString();

    bSettings->setMode(mode);

    ui->bareer_label->setText(QString::number(mode.bareerNumber));

    ui->exit_frame->setVisible(!mode.mode);
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
    settings.setValue("cameraIP",mode.cameraIP);
}

void MainWindow::openBareer()
{
    digitalWrite(BAREER_PIN,HIGH);
    delay(500);
    digitalWrite(BAREER_PIN,LOW);
}

double MainWindow::calculate_formula(const QString &formula, const quint64 &secs)
{
    double mins = secs/60;
    if(!formula.contains(':'))
        return mins*formula.toDouble()/60;

    QStringList pair;
    QStringList priceList = formula.split(';',QString::SkipEmptyParts);
    foreach (QString str, priceList) {
        pair = str.split(':',QString::SkipEmptyParts);
        if(mins<=QString(pair[0]).toDouble()){
            return QString(pair[1]).toDouble();
        }
    }
    return QString(pair[1]).toDouble();
}
