#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMessageBox>

#include <QDateTime>

#include "CustomServer.h"
#include <QTcpSocket>

#include <QJsonDocument>
#include <QJsonArray>

#include <QDebug>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>

#include <QHostInfo>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bDb(DatabaseManager::instance()),
    bSettings(new SettingsDialog(this)),
    curCode("")
{
    ui->setupUi(this);

    readSettings();
    initConnections();

    QList<QHostAddress> hostList = QHostInfo::fromName(QHostInfo::localHostName()).addresses();
    foreach (const QHostAddress& address, hostList) {
        qDebug()<<address;

        if (address.protocol() == QAbstractSocket::IPv4Protocol && address.isLoopback() == false) {
            localHostIP = address.toString();
        }
    }

    CustomServer *server = new CustomServer(this);

    server->setMaxPendingConnections(4);
    if(server->listen(QHostAddress::AnyIPv4, 1234)){
        connect(server, &CustomServer::newConnection, [this,server]{

            qDebug()<<"New connection";
            QTcpSocket *socket = server->nextPendingConnection();

            connect(socket, &QTcpSocket::readyRead, [this,socket,server]{
                QByteArray arr = server->sockets.value(socket->socketDescriptor()) + socket->readAll();
                //                qDebug()<<arr;

                if(!arr.endsWith("\"}")){
                    server->sockets.insert(socket->socketDescriptor(),arr);
                    //                    qDebug()<<"continue_fucking";
                    socket->write("HTTP/1.1 100 Continue\r\n\r\n");
                    return;
                }

                arr = arr.mid(arr.indexOf('{'));

                //                qDebug()<<"again"<<arr.data();
                QJsonDocument doc;
                QJsonObject obj;
                QJsonParseError error;

                doc = QJsonDocument::fromJson(arr,&error);

                if(error.error != QJsonParseError::NoError){
                    qDebug()<<error.errorString();
                    socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
                    return;
                }
                obj = doc.object();
                QString img_url = obj["site_id"].toString()+obj["uuid"].toString()+".jpg";
                QString code = obj["results"].toArray().first().toObject()["plate"].toString();

                if(curCode==code and bTimer.isActive()){
                    bTimer.start();
                }
                else {
                    curCode=code;
                    proceedCode(code,img_url,true);
                }
                socket->write("HTTP/1.1 200 OK\r\n\r\n");
                socket->close();
            });
            connect(socket, &QTcpSocket::disconnected, [socket,server]{
                qDebug()<<"socket deleted";
                server->sockets.remove(socket->socketDescriptor());
                socket->deleteLater();
            });

            if(!socket->waitForReadyRead(500)){
                socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
                socket->close();
            }

        });
    }
    else {
        qDebug()<<"Couldn't start server";
        exit(EXIT_FAILURE);
    }

    movie.setFileName(":/images/anim.gif");
    bTimer.setInterval(5000);
    cleanupInterface();

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

void MainWindow::proceedCode(const QString &code, const QString &img_url, bool plateMode)
{
    movie.stop();
    bTimer.stop();

    QSqlQuery query;

    if(!query.exec(QString("SELECT SYSDATE();"))){
        bDb.debugQuery(query);
        return;
    }

    query.next();

    QDateTime b_out_time = query.value("SYSDATE()").toDateTime();

    if(!query.exec(QString("SELECT id, access_type, in_number, in_time "
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
            QString access_type = query.value("access_type").toString();
            if(access_type=="Оплата"){
                ui->plate_number_label->setText("Уже зарегистрирован!");
                if(plateMode)openBareer();
            }
            else if(access_type=="Допуск"){
                ui->plate_number_label->setText("Уже допущен!");
                if(plateMode)openBareer();
            }
            else if(access_type=="Запрет")
                ui->plate_number_label->setText("Запрет!");

            ui->enter_time_label->setText(query.value("in_time").toDateTime().toString("dd.MM.yyyy H:mm"));
            ui->enter_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

            bTimer.start();

            return;
        }

        if(!query.exec(QString("SELECT access_type FROM Reserved_codes WHERE code='%1';").arg(code))){
            bDb.debugQuery(query);
            return;
        }
        query.next();
        QString access_type = query.isValid()?query.value("access_type").toString():"Оплата";

        query.prepare("INSERT INTO Active(`code`,`code_type`,`access_type`,`in_number`,`in_time`, `img`) "
                      "VALUES(?,?,?,?,?,?);");
        query.addBindValue(code);
        query.addBindValue(plateMode?"Номер":"Карта");
        query.addBindValue(access_type);
        query.addBindValue(bSettings->modeSettings().bareerNumber);
        query.addBindValue(b_out_time);

        //    ??????????????????????????????????????????????
        if(plateMode)
            query.addBindValue(img_url);
        else{
            QString filename = QString("%1_%2.jpg").arg(code).arg(b_out_time.currentMSecsSinceEpoch());
            query.addBindValue("http://"+localHostIP+"/plateimages/"+filename);
            QtConcurrent::run(this, &MainWindow::grubImg,
                              QUrl("http://"+bSettings->modeSettings().cameraIP+"/webcapture.jpg?command=snap&channel=1"),
                              QString(filename));
        }
        //  ?????????????

        if(!query.exec()){
            bDb.debugQuery(query);
            return;
        }

        ui->label->setText(QString("<p align=\"center\"><span style=\"font-size:100pt; color:#005500;\">%1</span></p>")
                           .arg(code));
        if(access_type=="Оплата"){
            ui->plate_number_label->setText("Успешно!");
            openBareer();
        }
        else if(access_type=="Допуск"){
            ui->plate_number_label->setText("Допуск!");
            openBareer();
        }
        else if(access_type=="Запрет")
            ui->plate_number_label->setText("Запрет!");

        ui->enter_time_label->setText(b_out_time.toString("dd.MM.yyyy H:mm"));
        ui->enter_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

        bTimer.start();

        return;
    }

    //EXIT
    if(!query.isValid()){
        if(!query.exec(QString("SELECT access_type, in_number, in_time, out_time, price "
                               "FROM History "
                               "WHERE out_time >= DATE_SUB(NOW() , INTERVAL 5 MINUTE) "
                               "AND code='%1' LIMIT 1")
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
        QString access_type = query.value("access_type").toString();

        int diff = in_time.time().secsTo(out_time.time());
        int hours = diff/3600;
        int minutes = (diff%3600)/60;
        int secs = (diff%3600)%60;
        QTime time(hours,minutes,secs);

        ui->plate_number_label->setText(QString("%1 : уже деактивирован!").arg(code));

        ui->enter_time_label->setText(in_time.toString("dd.MM.yyyy H:mm"));
        ui->enter_bareer_label->setText(query.value("in_number").toString());
        ui->exit_time_label->setText(out_time.toString("dd.MM.yyyy H:mm"));
        ui->exit_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

        if(access_type=="Оплата"){
            ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:70pt;\">%1</span></p>"
                                       "<p align=\"center\"><span style=\" font-size:150pt; color:#ea0003;\">%2 </span><span style=\" font-size:48pt; color:#ea0003;\">(UZS)</span></p>")
                               .arg(time.toString()).arg(query.value("price").toString()));
            openBareer();
        }
        else if(access_type=="Допуск"){
            ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                                       "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2</span></p>")
                               .arg(time.toString()).arg(access_type));
            openBareer();
        }
        else {
            ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                                       "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2</span></p>")
                               .arg(time.toString()).arg(access_type));
        }

        bTimer.start();
        return;
    }

    int b_id = query.value("id").toInt();
    QDateTime b_in_time = query.value("in_time").toDateTime();
    quint32  b_in_number = query.value("in_number").toUInt();

    if(!query.exec(QString("SELECT Price.price_formula, Reserved_codes.access_type "
                           "FROM Price INNER JOIN Reserved_codes ON Price.id = Reserved_codes.price_id "
                           "WHERE Reserved_codes.code = '%1' LIMIT 1;")
                   .arg(code))){
        bDb.debugQuery(query);
        return;
    }

    query.next();

    QString access_type="Оплата";

    if(!query.isValid()){

        if(!query.exec("SELECT price_formula FROM Price WHERE car_type = 'Другое'")){
            bDb.debugQuery(query);
            return;
        }
        query.next();
    }
    else access_type=query.value("access_type").toString();

    double price = calculate_formula(query.value("price_formula").toString(),
                                     b_in_time.secsTo(b_out_time));

    QString filename;
    if(!plateMode)
    {
        filename = QString("%1_%2.jpg").arg(code).arg(b_out_time.currentMSecsSinceEpoch());
        QtConcurrent::run(this, &MainWindow::grubImg,
                          QUrl("http://"+bSettings->modeSettings().cameraIP+"/webcapture.jpg?command=snap&channel=1"),
                          QString(filename));
    }
    if(!query.exec(QString("CALL MoveToHistory('%1','%2','%3','%4');")
                   .arg(b_id)
                   .arg(price)
                   .arg(bSettings->modeSettings().bareerNumber)
                   ///////////////////////////////
                   .arg(plateMode?img_url:"http://"+localHostIP+"/plateimages/"+filename))){
        bDb.debugQuery(query);
        return;
    }

    int diff = b_in_time.time().secsTo(b_out_time.time());
    int hours = diff/3600;
    int minutes = (diff%3600)/60;
    int secs = (diff%3600)%60;
    QTime time(hours,minutes,secs);

    ui->plate_number_label->setText(QString("%1 : Успешно деактивирован!").arg(code));

    ui->enter_time_label->setText(b_in_time.toString("dd.MM.yyyy H:mm"));
    ui->enter_bareer_label->setText(QString::number(b_in_number));
    ui->exit_time_label->setText(b_out_time.toString("dd.MM.yyyy H:mm"));
    ui->exit_bareer_label->setText(QString::number(bSettings->modeSettings().bareerNumber));

    if(access_type=="Оплата"){
        ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:70pt;\">%1</span></p>"
                                   "<p align=\"center\"><span style=\" font-size:150pt; color:#ea0003;\">%2 </span><span style=\" font-size:48pt; color:#ea0003;\">(UZS)</span></p>")
                           .arg(time.toString()).arg(price));
        print(code,time.toString(),price,b_in_time, b_out_time, b_in_number);
        openBareer();
    }
    else if(access_type=="Допуск"){
        ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                                   "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2 </span></p>")
                           .arg(time.toString()).arg(access_type));
        openBareer();
    }
    else {
        ui->label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                                   "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2 </span></p>")
                           .arg(time.toString()).arg(access_type));
    }

    bTimer.start();
}

bool MainWindow::grubImg(const QUrl &url, const QString &filename)
{
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, [filename,reply]{
        QFile file("/var/www/html/plateimages/"+filename);
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        reply->deleteLater();
    });
    return reply->waitForReadyRead(1000);
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

void MainWindow::initConnections()
{
    connect(ui->configureButton, &QPushButton::clicked, bSettings, &SettingsDialog::show);
    connect(ui->aboutButton, &QPushButton::clicked, [this](){
        QMessageBox::about(this, tr("About ParkingACS"),
                           tr("The <b>ParkingACS</b> is an Access Controll System for car parks!"                          "<p align='right'>Author: <b>Nurnazarov Bakhmanyor Yunuszoda</b></p> <a align='right' href="
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

    connect(&bDb,&DatabaseManager::connectionChanged, [this](bool status){
        if(status){
            ui->statusBar->showMessage("Successfully connected!");
        }
        else exit(EXIT_FAILURE);
    });

    connect(&bTimer, &QTimer::timeout, this, &MainWindow::cleanupInterface);
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

void MainWindow::cleanupInterface()
{
    ui->label->setMovie(&movie);
    movie.start();

    ui->enter_time_label->setText("");
    ui->enter_bareer_label->setText("");
    ui->plate_number_label->setText("Ожидание!");

    if(!bSettings->modeSettings().mode){
        ui->exit_time_label->setText("");
        ui->exit_bareer_label->setText("");
    }
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
