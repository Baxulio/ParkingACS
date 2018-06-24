#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

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

    struct ModeSettings {
        quint8 bareerNumber;
        bool mode;
        quint64 wait_time;
    };

    explicit SettingsDialog(QWidget *parent = nullptr,
                            ServerSettings server = ServerSettings{QString("localhost"),
                            QString("root"),
                            QString(""),
                            quint32(3306)},
                            ModeSettings mode = ModeSettings{quint8(1),bool(true),quint64(30)});
    ~SettingsDialog();

    ServerSettings serverSettings() const {return bServerSettings;}
    void setServerSettings(const ServerSettings server);

    ModeSettings modeSettings() const {return bMode;}
    void setMode(const ModeSettings mode);

private:
    Ui::SettingsDialog *ui = nullptr;
    ServerSettings bServerSettings;
    ModeSettings bMode;

private slots:
    void apply();

private:
    void updateSettings();
};

#endif // SETTINGSDIALOG_H
