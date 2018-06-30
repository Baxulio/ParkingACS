#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QMessageBox>

#include <QDateTime>

#include <QPainter>
#include <QDebug>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bSettings(new SettingsDialog),
    bsocket(new QTcpSocket(this)),
    label(new QLabel(this)),
    bTimer(0)
{
    ui->setupUi(this);

    ui->statusBar->addPermanentWidget(label);

    readSettings();
    initActionsConnections();

    connect(&timer, &QTimer::timeout, this, &MainWindow::makeConnection);
    connect(bsocket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
    connect(bsocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displaySocketError(QAbstractSocket::SocketError)));

    makeConnection();
}

MainWindow::~MainWindow()
{
    delete bSettings;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //if (maybeSave()) {
    writeSettings();
    event->accept();
    //} else {
    //   event->ignore();
    //}
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Parking"),
                       tr("The <b>Parking</b> is an automated parking system "
                          "with the following functionalities:"
                          "<ul><li>Capture an incoming car</li>"
                          "<li>Work with Wiegand devices</li>"
                          "<li>Run on any platform</li>"
                          "<li>Controll the barrier<li>"
                          "<li>Print cheque respect to minutes<li></ul>"
                          "<p align='right'>Author: <b>Nurnazarov Bakhmanyor Yunuszoda</b></p> <a align='right' href="
                          "'http://bahman.byethost18.com'>[bahman.byethost18.com]</a>"));
}

void MainWindow::showStatusMessage(const QString &message)
{
    label->setText(message);
}

void MainWindow::makeConnection()
{
    ui->in_out_number_label->setText("");
    ui->in_time_label->setText("");
    ui->out_time_label->setText("");

    //////////
    //SERVER//
    //////////

    bsocket->connectToHost(bSettings->serverSettings().host,bSettings->serverSettings().port);
    if(!bsocket->waitForConnected(5000)){
        ui->icon_label->setText("<p align=\"center\"><img src=\":/images/warning.png\"/><br/></p><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#ca7b29;\">НЕТ ПОДКЛЮЧЕНИЯ!</span></p>");
        ui->info_label->setText("<span style=\"font-size:26pt; color:#7e7e7e;\">Не удалось наладить сеть!");
        makeDisconnection();
        return;
    }

    setUpServer();

    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(true);
    label->setText("");
    bTimer=0;
    timer.stop();
}

void MainWindow::makeDisconnection()
{
    bTimer+=1000;
    bsocket->abort();

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);

    //ui->icon_label->setText("<p align=\"center\"><img src=\":/images/warning.png\"/><br/></p><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#ca7b29;\">НЕТ ПОДКЛЮЧЕНИЯ!</span></p>");
    //ui->info_label->setText("<span style=\"font-size:26pt; color:#7e7e7e;\">Не удалось наладить сеть!");

    timer.start(bTimer);
}

void MainWindow::wiegandCallback(quint32 value)
{
    QByteArray Buffer;
    QDataStream out(&Buffer,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);

    out<<false<<value;

    bWiegand = value;
    bsocket->write(Buffer);
    if(!bsocket->waitForReadyRead(5000)){
        makeDisconnection();
    }
    ui->info_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Код карты : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(value));
}

void MainWindow::readSocket()
{
    QByteArray arr=bsocket->readAll();

    QDataStream in(&arr,QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_5);
    if(in.status()==QDataStream::ReadCorruptData){
        ui->icon_label->setText("<img src=\":/images/warning.png\"/><br/><span style=\" font-size:36pt; font-weight:600; color:#ca7b29;\">НЕУДАЧА!</span>");
        ui->info_label->setText("<span style=\"font-size:26pt; color:#7e7e7e;\">Повторите попытку !");
    }
    int replyNumber;
    in >> replyNumber;

    switch (replyNumber)
    {
    //ENTER mode
    case Replies::WIEGAND_REGISTERED:{
        ui->icon_label->setText("<img src=\":/images/149_check_ok-512.png\"/><br/><span style=\" font-size:36pt; font-weight:600; color:#8dc63f;\">УСПЕШНО!</span>");
        ui->in_time_label->setText("");
        ui->in_out_number_label->setText((""));
        openBareer();
        return;
    }
    case Replies::WIEGAND_ALREADY_REGISTERED:{
        QDateTime in_time;
        quint8 in_number;

        in>>in_time>>in_number;
        ui->icon_label->setText("<img src=\":/images/warning.png\"/><br/></p><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#ca7b29;\">УЖЕ ЗАРЕГИСТРИРОВАН!</span>");
        ui->in_time_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Время въезда : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_time.toString("dd.MM.yyyy H:mm")));
        ui->in_out_number_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Въезд № : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_number));
        openBareer();
        return;
    }
        //EXIT mode
    case Replies::WIEGAND_IS_MONTHLY:{
        QDateTime in_time;
        QDateTime out_time;
        quint8 in_number;

        in>>in_time>>in_number>>out_time;
        ui->icon_label->setText(QString("<p align=\"center\"><br/></p><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#8dc63f;\">Абонемент!</span></p>"));

        ui->in_time_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Время въезда : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_time.toString("dd.MM.yyyy H:mm")));
        ui->out_time_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Время выезда : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(out_time.toString("dd.MM.yyyy H:mm")));
        ui->in_out_number_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Въезд № : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_number));
        openBareer();
        return;
    }

    case Replies::WIEGAND_DEACTIVATED:{
        qDebug()<<"deactivated";
        QDateTime in_time;
        QDateTime out_time;
        quint8 in_number;
        double price;


        in>>in_time>>in_number>>out_time>>price;

        int diff = in_time.time().secsTo(out_time.time());
        int hours = diff/3600;
        int minutes = (diff%3600)/60;
        int secs = (diff%3600)%60;
        QTime time(hours,minutes,secs);

        ui->icon_label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                                        "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2 </span><span style=\" font-size:48pt; color:#ea0003;\">(UZS)</span></p>"
                                        "<p align=\"center\"><span style=\" font-size:50pt; font-weight:600; color:#8dc63f;\">Карта деактивирована!</span>")
                                .arg(time.toString()).arg(price));



        ui->in_time_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Время въезда : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_time.toString("dd.MM.yyyy H:mm")));
        ui->out_time_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Время выезда : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(out_time.toString("dd.MM.yyyy H:mm")));
        ui->in_out_number_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Въезд № : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_number));

        print(time.toString(),price,in_time, out_time, in_number);
        openBareer();
        return;
    }
    case Replies::WIEGAND_ALREADY_DEACTIVATED:{
        qDebug()<<"already deactivated";
        QDateTime in_time;
        QDateTime out_time;
        quint8 in_number;
        double price;


        in>>in_time>>in_number>>out_time>>price;

        int diff = in_time.time().secsTo(out_time.time());
        int hours = diff/3600;
        int minutes = (diff%3600)/60;
        int secs = (diff%3600)%60;
        QTime time(hours,minutes,secs);

        ui->icon_label->setText(QString("<p align=\"center\"><span style=\" font-size:100pt;\">%1</span></p>"
                                        "<p align=\"center\"><span style=\" font-size:250pt; color:#ea0003;\">%2 </span><span style=\" font-size:48pt; color:#ea0003;\">(UZS)</span></p>"
                                        "<p align=\"center\"><span style=\" font-size:50pt; font-weight:600; color:#7e7e7e;\">Карта уже деактивирована!</span>")
                                .arg(time.toString()).arg(price));


        ui->in_time_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Время въезда : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_time.toString("dd.MM.yyyy H:mm")));
        ui->out_time_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Время выезда : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(out_time.toString("dd.MM.yyyy H:mm")));
        ui->in_out_number_label->setText(QString("<span style=\" font-size:26pt; color:#7e7e7e;\">Въезд № : </span><span style=\" font-size:26pt; font-weight:600; color:#7e7e7e;\">%1</span>").arg(in_number));

        openBareer();
        return;
    }
    case Replies::WIEGAND_NOT_REGISTERED:{
        ui->icon_label->setText("<img src=\":/images/warning.png\"/><br/></p><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#ca7b29;\">ОШИБКА!</span>"
                                "<p align=\"center\"><span style=\"font-size:26pt; color:#7e7e7e;\">Карта не зафиксирована при въезде!");

        ui->in_time_label->setText("");
        ui->out_time_label->setText("");
        ui->in_out_number_label->setText("");
        return;
    }
        //Setting up
    case Replies::SET_UP:{
        ui->icon_label->setText("<img src=\":/images/149_check_ok-512.png\"/><br/><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#8dc63f;\">ПОДКЛЮЧЕНО!</span>");
        ui->info_label->setText("<span style=\"font-size:26pt; color:#7e7e7e;\">Приложите карту!");
        return;
    }
    case Replies::DVR_ERROR:{
        ui->icon_label->setText("<img src=\":/images/149_check_ok-512.png\"/><br/><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#8dc63f;\">ПОДКЛЮЧЕНО БЕЗ КАМЕРЫ!</span>");
        ui->info_label->setText("<span style=\"font-size:26pt; color:#7e7e7e;\">Приложите карту!");
        return;
    }
        //    case Replies::SNAPSHOT_FAIL:{
        //        ui->icon_label->setText("<img src=\":/images/warning.png\"/><br/><p align=\"center\"><span style=\" font-size:36pt; font-weight:600; color:#ca7b29;\">НЕУДАЧА!</span>");
        //        ui->info_label->setText("<span style=\"font-size:26pt; color:#7e7e7e;\">Не удалось снять на камеру!");
        //        makeDisconnection();
        //        return;
        //    }
    default:{
        makeDisconnection();
    }
    }
}

void MainWindow::displaySocketError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        showStatusMessage("<font color='red'>Server closed!");
        break;
    case QAbstractSocket::HostNotFoundError:
        showStatusMessage("<font color='red'>The host was not found. Please check the host name and port settings!");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        showStatusMessage("<font color='red'>The connection was refused by the peer!");
        break;
    default:
        showStatusMessage(QString("<font color='red'>%1!").arg(bsocket->errorString()));
    }
    makeDisconnection();
}

void MainWindow::print(const QString &dur, double price, const QDateTime &in_time, const  QDateTime &out_time, const quint32 in)
{
    char t[1000];
    sprintf(t,"printf '********************************\nStroy Gorod [%02d]\n\nKarta kodi: %d\nKirish vaqti: %s\nChiqish vaqti: %s\nTurgan vaqti: %s\nPul miqdori: %.2f\nKirish #: %d\n********** powered by GSS.UZ\n\n\n\n' >/dev/usb/lp0",
            bSettings->modeSettings().bareerNumber,
            bWiegand,
            in_time.toString("dd.MM.yyyy H:mm").toLatin1().data(),
            out_time.toString("dd.MM.yyyy H:mm").toLatin1().data(),
            dur.toLatin1().data(),
            price,
            in
            );
    system(t);
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered, bSettings, &SettingsDialog::show);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionConnect,&QAction::triggered, this, &MainWindow::makeConnection);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::makeDisconnection);
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

    if(!settings.contains("server_host")){
        return;
    }
    SettingsDialog::ServerSettings server;
    server.host = settings.value("server_host",QString()).toString();
    server.user = settings.value("server_user",QString()).toString();
    server.password = settings.value("server_password",QString()).toString();
    server.port = settings.value("server_port",quint32()).toUInt();

    bSettings->setServerSettings(server);

    SettingsDialog::ModeSettings mode;
    mode.mode = settings.value("mode",bool()).toBool();
    mode.bareerNumber = settings.value("bareer_number",quint8()).toUInt();

    bSettings->setModeSettings(mode);


    SettingsDialog::DVRSettings dvr;
    dvr.host = settings.value("dvr_host",QString()).toString();
    dvr.user = settings.value("dvr_user",QString()).toString();
    dvr.password = settings.value("dvr_password",QString()).toString();
    dvr.port = settings.value("dvr_port",quint32()).toUInt();
    dvr.connectionMethod = settings.value("dvr_connectionMethod",quint8()).toUInt();

    bSettings->setDvrSettings(dvr);
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
    settings.setValue("mode", mode.mode);
    settings.setValue("bareer_number",mode.bareerNumber);

    SettingsDialog::DVRSettings dvr = bSettings->dvrSettings();
    settings.setValue("dvr_host",dvr.host);
    settings.setValue("dvr_user",dvr.user);
    settings.setValue("dvr_password",dvr.password);
    settings.setValue("dvr_port",dvr.port);
    settings.setValue("dvr_connectionMethod",dvr.connectionMethod);
}

void MainWindow::openBareer()
{
    digitalWrite(BAREER_PIN,HIGH);
    delay(500);
    digitalWrite(BAREER_PIN,LOW);
}

void MainWindow::setUpServer()
{
    QByteArray Buffer;
    QDataStream out(&Buffer,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);

    out<<true
      <<bSettings->modeSettings().bareerNumber
     <<bSettings->modeSettings().mode
    <<bSettings->dvrSettings().host;

    bsocket->write(Buffer);
    if(!bsocket->waitForReadyRead(5000)){
        makeDisconnection();
        return;
    }
    ui->bareer_label->setText(QString("<p><span style=\" font-size:14pt; color:#7e7e7e;\">%1 : "
                                      "</span><span style=\" font-size:14pt; font-weight:600; color:#7e7e7e;\">%2</span></p>").arg(bSettings->modeSettings().mode?"Въезд #":"Выезд #").
                              arg(bSettings->modeSettings().bareerNumber));
}
