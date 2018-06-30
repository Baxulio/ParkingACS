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
        bool mode;
        quint8 bareerNumber;
    };
    struct DVRSettings {
        QString host;
        QString user;
        QString password;
        quint32 port;
        quint8 connectionMethod;
    };

    explicit SettingsDialog(ServerSettings server = ServerSettings{QString("ParkingServer"),QString("ParkingUser"),QString(""),quint32(3306)},
                            ModeSettings mode = ModeSettings{true,1},
                            DVRSettings dvr = DVRSettings{QString("DVR1"), QString("DVRUser"),QString(""),quint32(34567),quint8(0)},
                            QWidget *parent = nullptr);
    ~SettingsDialog();

    ServerSettings serverSettings() const {return bServerSettings;}
    void setServerSettings(const ServerSettings server);

    ModeSettings modeSettings() const {return bModeSettings;}
    void setModeSettings(const ModeSettings mode);

    DVRSettings dvrSettings() const {return bDVRSettings;}
    void setDvrSettings(const DVRSettings dvr);

private:
    Ui::SettingsDialog *ui = nullptr;
    ServerSettings bServerSettings;
    ModeSettings bModeSettings;
    DVRSettings bDVRSettings;

private slots:
    void apply();

private:
    void updateSettings();
};

#endif // SETTINGSDIALOG_H
