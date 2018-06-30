#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DatabaseManager.h>
#include <QSqlRecord>

#include "dialogs/SettingsDialog.h"
#include "Core.h"

#include "wiringPi.h"

#include <QLabel>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DatabaseManager &bDb;

    SettingsDialog *bSettings;
    QTimer bTimer;


protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void about();
    void makeConnection();
    void makeDisconnection();

    void print(const QString &dur, double price, const QDateTime &in_time, const QDateTime &out_time, const quint32 in);

public slots:
    void wiegandCallback(quint32 value);
    void showStatusMessage(const QString &message);

private:
    void initActionsConnections();

    void readSettings();
    void writeSettings();

    void openBareer();
};

#endif // MAINWINDOW_H
