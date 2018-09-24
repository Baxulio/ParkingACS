#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DatabaseManager.h>

#include "dialogs/SettingsDialog.h"
#include "Core.h"

#include "alpr.h"

#include "wiringPi.h"

#include <QTimer>
#include <QMovie>

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
    QMovie movie;

    QString curCode;
    QString localHostIP;
protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void proceedCode(const QString &code, const QString &img_url = QString(), bool plateMode=false);

private slots:
    void initConnections();

    void readSettings();
    void writeSettings();

    void cleanupInterface();
    void openBareer();
    void print(const QString &code, const QString &dur, double price, const QDateTime &in_time, const QDateTime &out_time, const quint32 in);
    double calculate_formula(const QString &formula, const quint64 &secs);

signals:
    void grub(QString filename, QString urls);
};

#endif // MAINWINDOW_H
