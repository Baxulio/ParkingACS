#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, SettingsDialog::ServerSettings server, SettingsDialog::ModeSettings mode):
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    bServerSettings(server),
    bMode(mode)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::apply);
}

void SettingsDialog::apply(){

    updateSettings();
    hide();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setServerSettings(const SettingsDialog::ServerSettings server)
{
    bServerSettings = server;
    ui->server_hostname_line_edit->setText(bServerSettings.host);
    ui->server_password_line_edit->setText(bServerSettings.password);
    ui->server_port_spin_box->setValue(bServerSettings.port);
    ui->server_login_line_edit->setText(bServerSettings.user);
}

void SettingsDialog::setMode(const SettingsDialog::ModeSettings mode)
{
    bMode = mode;
    ui->barrier_number_spin->setValue(bMode.bareerNumber);
    bMode.mode?ui->enterRadio->setChecked(true):ui->exitRadio->setChecked(true);
    ui->wait_time_spin->setValue(bMode.wait_time);
}

void SettingsDialog::updateSettings()
{
    bServerSettings.host = ui->server_hostname_line_edit->text();
    bServerSettings.password = ui->server_password_line_edit->text();
    bServerSettings.port = ui->server_port_spin_box->value();
    bServerSettings.user = ui->server_login_line_edit->text();

    bMode.bareerNumber = ui->barrier_number_spin->value();
    bMode.mode = ui->enterRadio->isChecked()?true:false;
    bMode.wait_time = ui->wait_time_spin->value();
}
