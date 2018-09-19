#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMenu>
#include <QMessageBox>

#include "dialogs/SettingsDialog.h"
#include "delegates/SlidingStackedWidget.h"

#include "forms/CurrentForm.h"
#include "forms/HistoryForm.h"
#include "forms/AccessParametersForm.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bDb(DatabaseManager::instance()),
    bSettings(new SettingsDialog(this)),
    bStackedWidget(new SlidingStackedWidget(this))
{
    ui->setupUi(this);
    ui->stack_widget_layout->insertWidget(0,bStackedWidget,1);
    bStackedWidget->setSpeed(370);
    connect(bStackedWidget, &SlidingStackedWidget::currentChanged, [this](int n){
        if(qobject_cast<CurrentForm*>(bStackedWidget->widget(n)))
            ui->current_but->setChecked(true);
        else if(qobject_cast<HistoryForm*>(bStackedWidget->widget(n)))
            ui->history_but->setChecked(true);
        else if(qobject_cast<AccessParametersForm*>(bStackedWidget->widget(n)))
            ui->access_params_but->setChecked(true);

        emit ui->autoUpdate_button->clicked();
    });

    QTimer *timer2 = new QTimer(this);
    connect(timer2, &QTimer::timeout, [this]{
        this->ui->status_label->setText("");
    });
    connect(this, &MainWindow::showStatusMessage, [this,timer2](const QString &msg){
        this->ui->status_label->setText(msg);
        timer2->start(5000);
    });

    connect(&timer, &QTimer::timeout, [this](){emit ui->autoUpdate_button->clicked();});

    readSettings();
    initActionsConnections();

    connect(&bDb,&DatabaseManager::connectionChanged, [this](bool status){
        if(status){
            emit showStatusMessage("<font color='green'>Successfully connected!");
            emit ui->autoUpdate_button->clicked();
        }
        else emit showStatusMessage(QString("<font color='red'>%1 </font><font color='grey'>Disconnected!").arg(bDb.lastError().databaseText()));

        ui->connectButton->setEnabled(!status);
        ui->disconnectButton->setEnabled(status);
        ui->splitter->setEnabled(status);
    });

    ui->current_but->toggled(true);
    ui->connectButton->clicked();
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

void MainWindow::initActionsConnections()
{
    /////////////////Autoupdate///////////////////////////////////////
    /// \brief autoUpdate_menu
    ///
    QMenu *autoUpdate_menu = new QMenu("Автообновление", this);
    autoUpdate_menu->addAction(ui->action_disabled_autorefresh);
    autoUpdate_menu->addAction(ui->action_autorefresh_1_sec);
    autoUpdate_menu->addAction(ui->action_autorefresh_5_sec);
    autoUpdate_menu->addAction(ui->action_autorefresh_10_sec);
    autoUpdate_menu->addAction(ui->action_autorefresh_20_sec);
    autoUpdate_menu->addAction(ui->action_autorefresh_35_sec);
    autoUpdate_menu->addAction(ui->action_autorefresh_1_min);

    connect(ui->action_autorefresh_1_sec, &QAction::triggered, [this](){
        ui->autoUpdate_button->setText(ui->action_autorefresh_1_sec->text());
        timer.start(1000);
    });
    connect(ui->action_autorefresh_5_sec, &QAction::triggered, [this](){
        ui->autoUpdate_button->setText(ui->action_autorefresh_5_sec->text());
        timer.start(5000);
    });
    connect(ui->action_autorefresh_10_sec, &QAction::triggered, [this](){
        ui->autoUpdate_button->setText(ui->action_autorefresh_10_sec->text());
        timer.start(10000);
    });
    connect(ui->action_autorefresh_20_sec, &QAction::triggered, [this](){
        ui->autoUpdate_button->setText(ui->action_autorefresh_20_sec->text());
        timer.start(20000);
    });
    connect(ui->action_autorefresh_35_sec, &QAction::triggered, [this](){
        ui->autoUpdate_button->setText(ui->action_autorefresh_35_sec->text());
        timer.start(35000);
    });
    connect(ui->action_autorefresh_1_min, &QAction::triggered, [this](){
        ui->autoUpdate_button->setText(ui->action_autorefresh_1_min->text());
        timer.start(60000);

    });
    connect(ui->action_disabled_autorefresh, &QAction::triggered, [this](){
        ui->autoUpdate_button->setText(ui->action_disabled_autorefresh->text());
        timer.stop();
    });
    ui->autoUpdate_button->setMenu(autoUpdate_menu);
    connect(ui->autoUpdate_button, &QToolButton::clicked, [this]{
        emit bDb.refresh();
    });
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    connect(ui->configureButton, &QPushButton::clicked, bSettings, &SettingsDialog::show);

    connect(ui->aboutButton, &QPushButton::clicked, [this](){
        QMessageBox::about(this, tr("About ParkingACS"),
                           tr("The <b>ParkingACS</b> is an Access Controll System for car parks!"
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

    connect(ui->current_but, &QPushButton::toggled, [this](bool checked){
        if(!checked)return;
        for (int i = 0; i < bStackedWidget->count(); i++) {
            if(qobject_cast<CurrentForm*>(bStackedWidget->widget(i))){
                bStackedWidget->slideInIdx(i/*,SlidingStackedWidget::t_direction::AUTOMATIC*/);
                return;
            }
        }
        CurrentForm *currentForm = new CurrentForm(this);
        bStackedWidget->slideInIdx(bStackedWidget->addWidget(currentForm));
        connect(currentForm, &CurrentForm::back, [this, currentForm](){
            if(bStackedWidget->count()<2)return;
            bStackedWidget->removeWidget(currentForm);
            currentForm->deleteLater();
        });
        connect(currentForm, &CurrentForm::message, this, &MainWindow::showStatusMessage);
    });
    connect(ui->history_but, &QPushButton::toggled, [this](bool checked){
        if(!checked)return;
        for (int i = 0; i < bStackedWidget->count(); i++) {
            if(qobject_cast<HistoryForm*>(bStackedWidget->widget(i))){
                bStackedWidget->slideInIdx(i/*,SlidingStackedWidget::t_direction::AUTOMATIC*/);
                return;
            }
        }
        HistoryForm *historyForm = new HistoryForm(this);
        bStackedWidget->slideInIdx(bStackedWidget->addWidget(historyForm));
        connect(historyForm, &HistoryForm::back, [this, historyForm](){
            if(bStackedWidget->count()<2)return;
            bStackedWidget->removeWidget(historyForm);
            historyForm->deleteLater();
        });
        connect(historyForm, &HistoryForm::message, this, &MainWindow::showStatusMessage);
    });
    connect(ui->access_params_but, &QPushButton::toggled, [this](bool checked){
        if(!checked)return;
        for (int i = 0; i < bStackedWidget->count(); i++) {
            if(qobject_cast<AccessParametersForm*>(bStackedWidget->widget(i))){
                bStackedWidget->slideInIdx(i/*,SlidingStackedWidget::t_direction::AUTOMATIC*/);
                return;
            }
        }
        AccessParametersForm *accessParametersForm = new AccessParametersForm(this);
        bStackedWidget->slideInIdx(bStackedWidget->addWidget(accessParametersForm));
        connect(accessParametersForm, &AccessParametersForm::back, [this, accessParametersForm](){
            if(bStackedWidget->count()<2)return;
            bStackedWidget->removeWidget(accessParametersForm);
            accessParametersForm->deleteLater();
        });
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

    if(!settings.contains("server_host")){
        return;
    }
    SettingsDialog::ServerSettings server;
    server.host = settings.value("server_host",QString()).toString();
    server.user = settings.value("server_user",QString()).toString();
    server.password = settings.value("server_password",QString()).toString();
    server.port = settings.value("server_port",quint32()).toUInt();

    bSettings->setServerSettings(server);
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
}
