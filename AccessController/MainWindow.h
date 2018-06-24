#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DatabaseManager.h>
#include <QSqlRecord>

#include "dialogs/SettingsDialog.h"
#include "Core.h"
#include "wiringPi.h"

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

    void wiegandCallback(quint32 code);

private:
    Ui::MainWindow *ui;
    DatabaseManager &bDb;

    SettingsDialog *bSettings;
    QTimer bTimer;

    QSqlRecord enterRec;
    QSqlRecord exitRec;


protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initActionsConnections();
    void readSettings();
    void writeSettings();

    bool enter(quint32 code);
    bool exit(quint32 code);

    void print(const QString &title, const QSqlRecord &record);
public slots:
    void interrupt();
    void timeout();
};

#endif // MAINWINDOW_H
