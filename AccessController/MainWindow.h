#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DatabaseManager.h>

#include "dialogs/SettingsDialog.h"
#include "Core.h"

#include "wiringPi.h"
#include "alpr.h"

#include <QLabel>
#include <QTimer>
#include <QMovie>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

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

    alpr::Alpr openalpr;
    alpr::AlprResults results;

    QNetworkAccessManager manager;
    QNetworkRequest request;
    QByteArray currentFrame;

    QTimer bTimer;
    QMovie movie;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void proceedCode(QString code, bool plateMode);
    void handleReply(QNetworkReply *pReply);

public slots:
    void wiegandCallback(quint32 value);
    void showStatusMessage(const QString &message);

private:
    void initActionsConnections();

    void readSettings();
    void writeSettings();

    void openBareer();
    void print(const QString &code, const QString &dur, double price, const QDateTime &in_time, const QDateTime &out_time, const quint32 in);
    double calculate_formula(const QString &formula, const quint64 &secs);
};

#endif // MAINWINDOW_H
