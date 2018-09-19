#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent):
    QDialog(parent),
    ui(new Ui::SettingsDialog)
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

void SettingsDialog::updateSettings()
{
    bServerSettings.host = ui->server_hostname_line_edit->text();
    bServerSettings.password = ui->server_password_line_edit->text();
    bServerSettings.port = ui->server_port_spin_box->value();
    bServerSettings.user = ui->server_login_line_edit->text();
}
