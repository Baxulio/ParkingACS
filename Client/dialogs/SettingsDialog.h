#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct ServerSettings {
        QString host;
        QString user;
        QString password;
        quint32 port;
    };

    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    ServerSettings serverSettings() const {return bServerSettings;}
    void setServerSettings(const ServerSettings server);

    QStringList urls();
    void setUrls(QStringList strl);

private:
    Ui::SettingsDialog *ui = nullptr;
    ServerSettings bServerSettings;
    QStringListModel *model;

private slots:
    void apply();

    void on_add_but_clicked();

    void on_del_but_clicked();

private:
    void updateSettings();
};

#endif // SETTINGSDIALOG_H
